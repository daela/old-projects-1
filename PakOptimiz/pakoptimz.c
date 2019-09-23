//system headers
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

//custom headers
#include "sha1.h"

#ifdef __i386__
#define stricmp strcasecmp
#endif

//define relevant q2 data types
typedef unsigned char 		byte;
typedef enum {false, true}	qboolean;

#define	MAX_QPATH			64		// max length of a quake game pathname
#define	MAX_OSPATH			128		// max length of a filesystem pathname

#define	MAX_FILES_IN_PACK	4096

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	packfile_t	*files;
} pack_t;

#define IDPAKHEADER		(('K'<<24)+('C'<<16)+('A'<<8)+'P')

typedef struct
{
	char	name[56];
	int		filepos, filelen;
} dpackfile_t;

typedef struct
{
	int		ident;		// == IDPAKHEADER
	int		dirofs;
	int		dirlen;
} dpackheader_t;

pack_t			*pak;
FILE			*out;
char			newname[MAX_OSPATH];

#define ERR_FATAL 1
#define	MAXPRINTMSG	4096

void Com_Error (int code, char *fmt, ...)
{
	va_list		argptr;
	static char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	printf ("ERROR: ");
	printf (msg);
	printf ("\n");

	if (out)
	{
		fclose (out);
		remove (newname);
	}

	exit (1);
}

void Com_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	printf (msg);
}

#define	Z_MAGIC		0x1d1d


typedef struct zhead_s
{
	struct zhead_s	*prev, *next;
	short	magic;
	short	tag;			// for group free
	int		size;
} zhead_t;

zhead_t		z_chain;
int		z_count, z_bytes;

/*
========================
Z_Free
========================
*/
void Z_Free (void *ptr)
{
	zhead_t	*z;

	z = ((zhead_t *)ptr) - 1;

	if (z->magic != Z_MAGIC)
		Com_Error (ERR_FATAL, "Z_Free: bad magic");

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;
	free (z);
}


/*
========================
Z_Stats_f
========================
*/
void Z_Stats_f (void)
{
	Com_Printf ("%i bytes in %i blocks\n", z_bytes, z_count);
}

/*
========================
Z_FreeTags
========================
*/
void Z_FreeTags (int tag)
{
	zhead_t	*z, *next;

	for (z=z_chain.next ; z != &z_chain ; z=next)
	{
		next = z->next;
		if (z->tag == tag)
			Z_Free ((void *)(z+1));
	}
}

/*
========================
Z_TagMalloc
========================
*/
void *Z_TagMalloc (int size, int tag)
{
	zhead_t	*z;
	
	size = size + sizeof(zhead_t);
	z = malloc(size);
	if (!z)
		Com_Error (ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes",size);
	memset (z, 0, size);
	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	return (void *)(z+1);
}

/*
========================
Z_Malloc
========================
*/
void *Z_Malloc (int size)
{
	return Z_TagMalloc (size, 0);
}

int Q_strncasecmp (char *s1, char *s2, int n)
{
	int		c1, c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;		// strings not equal
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int Q_strcasecmp (char *s1, char *s2)
{
	return Q_strncasecmp (s1, s2, 99999);
}

/*
================
FS_filelength
================
*/
int FS_filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

/*
==============
FS_FCloseFile

For some reason, other dll's can't just cal fclose()
on files returned by FS_FOpenFile...
==============
*/
void FS_FCloseFile (FILE *f)
{
	fclose (f);
}

/*
=================
FS_ReadFile

Properly handles partial reads
=================
*/
void CDAudio_Stop(void);
#define	MAX_READ	0x10000		// read in blocks of 64k
void FS_Read (void *buffer, int len, FILE *f)
{
	size_t	block;
	size_t	remaining;
	size_t	read;
	byte	*buf;
	int		tries;

	buf = (byte *)buffer;

	// read in chunks for progress bar
	remaining = len;
	tries = 0;
	while (remaining)
	{
		block = remaining;
		if (block > MAX_READ)
			block = MAX_READ;
		read = fread (buf, 1, block, f);
		if (read == 0)
		{
			// we might have been trying to read from a CD
			if (!tries)
			{
				tries = 1;
				//CDAudio_Stop();
			}
			else
				Com_Error (ERR_FATAL, "FS_Read: 0 bytes read");
		}

		if (read == -1)
			Com_Error (ERR_FATAL, "FS_Read: -1 bytes read");

		// do some progress bar thing here...

		remaining -= read;
		buf += read;
	}
}

int FS_FOpenFile (char *filename, FILE **file)
{
	int				i;

	for (i=0 ; i < pak->numfiles ; i++) {
		if (!Q_strcasecmp (pak->files[i].name, filename))
		{	// found it!
		// open a new file on the pakfile
			*file = fopen (pak->filename, "rb");
			if (!*file)
				Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename);	
			fseek (*file, pak->files[i].filepos, SEEK_SET);
			return pak->files[i].filelen;
		}
	}
	
	*file = NULL;
	return -1;
}

/*
============
FS_LoadFile

Filename are reletive to the quake search path
a null buffer will just return the file length without loading
============
*/
int FS_LoadFile (char *path, void **buffer)
{
	FILE	*h;
	byte	*buf;
	int		len;
	buf = NULL;	// quiet compiler warning

// look for it in the filesystem or pack files
	len = FS_FOpenFile (path, &h);
	if (!h)
	{
		if (buffer)
			*buffer = NULL;
		return -1;
	}
	
	if (!buffer)
	{
		fclose (h);
		return len;
	}

	buf = Z_Malloc(len);
	*buffer = buf;

	FS_Read (buf, len, h);

	fclose (h);

	return len;
}


/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile (void *buffer)
{
	Z_Free (buffer);
}

dpackheader_t	existing_header;
dpackfile_t		info[MAX_FILES_IN_PACK];

/*
=================
FS_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
pack_t *FS_LoadPackFile (char *packfile)
{
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	FILE			*packhandle;

	packhandle = fopen(packfile, "rb");
	if (!packhandle)
		return NULL;

	fread (&existing_header, 1, sizeof(existing_header), packhandle);
	if (existing_header.ident != IDPAKHEADER)
		Com_Error (ERR_FATAL, "%s is not a packfile", packfile);

	numpackfiles = existing_header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Com_Error (ERR_FATAL, "%s has %i files", packfile, numpackfiles);

	newfiles = Z_Malloc (numpackfiles * sizeof(packfile_t));

	fseek (packhandle, existing_header.dirofs, SEEK_SET);
	fread (info, 1, existing_header.dirlen, packhandle);

// parse the directory
	for (i=0 ; i<numpackfiles ; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = info[i].filepos;
		newfiles[i].filelen = info[i].filelen;
	}

	pack = Z_Malloc (sizeof (pack_t));
	strcpy (pack->filename, packfile);

	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;

	Com_Printf ("Loaded packfile %s (%i files)\n", packfile, numpackfiles);
	return pack;
}

int main(int argc, char* argv[])
{
	int i, len, size, totalin, totalout, j;
	int numfilesout;
	byte hashes[MAX_FILES_IN_PACK][20];
	//dpackfilez_t		newinfo[MAX_FILES_IN_PACK];
	dpackheader_t header;
	byte *filebuff;
	FILE *pack;
	SHA_CTX sha;

	out = NULL;

	printf ("pakoptimz 0.0.1 (c) 2004 r1ch.net\n");

	if (argc < 2) {
		printf ("usage: %s original.pak [new.pak]\n", argv[0]);
		return 1;
	}

	//init q2 memory allocation stuff
	z_chain.next = z_chain.prev = &z_chain;

	if (argc == 3 && !stricmp (argv[1], argv[2]))
		Com_Error (ERR_FATAL, "DONT DO THAT");

	pack = fopen (argv[1], "rb");
	if (!pack)
		Com_Error (ERR_FATAL, "Couldn't open %s", argv[1]);
	totalin = FS_filelength (pack);
	fclose (pack);

	//load the uncompressed pak
	pak = FS_LoadPackFile (argv[1]);

	if (!pak)
		Com_Error (ERR_FATAL, "Couldn't open %s", argv[1]);
		

	if (argc == 3)
	{
		strcpy (newname, argv[2]);
	}
	else
	{
		strcpy (newname, argv[1]);
		strcat (newname, ".tmp");
	}

	//attempt to open output
	if (!(out = fopen (newname, "wb")))
		Com_Error (ERR_FATAL, "Couldn't write to %s", newname);

	//skip over the header (we write it after the data)
	if (fseek (out, sizeof(header), SEEK_SET))
		Com_Error (ERR_FATAL, "Couldn't seek past header on %s", newname);

	printf ("\nCreating %s\n", newname);

	size = 0;

	//write the data before the header (weird but it works)
	for (i=0 ; i<pak->numfiles; i++)
	{
		len = FS_LoadFile (info[i].name, (void *)&filebuff);
		if (!len) {
			printf ("! Ignoring %s: 0 byte file.\n", info[i].name);
			info[i].filelen = 0;
			continue;
		}

		if (len == -1) {
			printf ("! Corrupt file: %s\n", info[i].name);
			info[i].filelen = 0;
			continue;
		}

		SHAInit (&sha);
		SHAUpdate (&sha, filebuff, info[i].filelen);
		SHAFinal (hashes[i], &sha);

		for (j = 0; j < i; j++)
		{
			if (!memcmp (hashes[i], hashes[j], 20))
			{
				if (info[i].filepos != info[j].filepos)
				{
					printf ("Saved %d bytes spotting duplicate '%s' of '%s'\n", info[i].filelen, info[i].name, info[j].name);
					info[i].filepos = info[j].filepos;
				}
				FS_FreeFile (filebuff);
				goto foo;
			}
		}

		info[i].filepos = size + sizeof(header);

		if (!(fwrite (filebuff, info[i].filelen, 1, out)))
			Com_Error (ERR_FATAL, "Error writing pak data.");

		FS_FreeFile (filebuff);

		size += info[i].filelen;
foo:;
	}

	FS_FCloseFile (pak->handle);

	printf ("Wrote file content (%d bytes).\n", size);

	numfilesout = i;

	//we clobber size later on so prepare this part of the header now
	header.dirofs = size + sizeof(header);

	size = pak->numfiles * sizeof(dpackfile_t);

	//prepare the directory
	printf ("Writing directory structure (%d bytes).\n", size);

	//write the directory
	if (!(fwrite (info, size, 1, out)))
		Com_Error (ERR_FATAL, "Error writing pak directory");

	//create the header (yes this a little weird writing the header AFTER
	//the data and directory, but we compress the directory hence need the
	//new dirlen...
	header.dirlen = size; //blocksize * sizeof(packfilez_t);
	header.ident = IDPAKHEADER;

	//back to the beginning...
	if (fseek (out, 0, SEEK_SET))
		Com_Error (ERR_FATAL, "Couldn't seek to write header");

	//write it out
	printf ("Writing pak header (%d bytes).\n", sizeof(header));

	if (!(fwrite (&header, sizeof(header), 1, out)))
		Com_Error (ERR_FATAL, "Couldn't write pak header");

	//status report
	totalout = FS_filelength (out);
	fclose (out);

	printf ("Completed. %d bytes -> %d bytes (%.2f%%)\n", totalin, totalout, ((float)totalout / (float)totalin) * 100.0f);

	if (argc == 2)
	{
		if (!remove(argv[1]))
		{
			if (rename (newname, argv[1]))
			{
				Com_Error (ERR_FATAL, "Couldn't rename temp file %s to %s.", newname, argv[1]);
			}
		}
		else
		{
			Com_Error (ERR_FATAL, "Couldn't delete existing file %s.", argv[1]);
		}
	}

	return 0;
}

