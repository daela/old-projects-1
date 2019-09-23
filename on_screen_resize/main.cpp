// License: Public Domain
// Author: nacitar sevaht
//
// g++ -lX11 -lXrandr -o on_screen_resize main.cpp
//
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/extensions/Xrandr.h>


int ErrorHandler(Display *d, XErrorEvent *error)
{
	if (error)
	{
		char buf[255];
		XGetErrorText(d,error->error_code,buf,sizeof(buf));
		std::cerr << "XError: " << +error->error_code << " / " << +error->request_code << " / " << +error->minor_code << ": " << buf << std::endl;
	}
	return 0;
}
int IOErrorHandler(Display *d)
{
	std::cerr << "X I/O Error: " << errno << std::endl;
	exit(1);
	return 1;
}

int main(int argc,char*argv[])
{
	if (argc < 2)
	{
		std::cerr << "usage: on_screen_resize <command...>" << std::endl;
		return 1;
	}
	std::vector<char*> vcToolArgs;
	vcToolArgs.push_back(argv[1]);
	for (int i=1;i<argc;++i)
	{
		vcToolArgs.push_back(argv[i]);
	}
	vcToolArgs.push_back(NULL);
	char**ptToolArgs=&vcToolArgs[0];
	try
	{
		Display *ptDisplay=XOpenDisplay(":0.0");
		if (!ptDisplay)
		{
			throw std::runtime_error("Failed to open display!");
		}
                // set up default error handlers
                XSetErrorHandler(ErrorHandler);
                XSetIOErrorHandler(IOErrorHandler);
		
		int event_base, error_base;
		if (!XRRQueryExtension(ptDisplay,&event_base,&error_base))
		{
			throw std::runtime_error("Xrandr is not available!");
		}
		// store default screen and root window
		int iDefaultScreen=XDefaultScreen(ptDisplay);
		int iDefaultRootWindow=XDefaultRootWindow(ptDisplay);

		// Register to get screen change notifications
		XRRSelectInput(ptDisplay,iDefaultRootWindow,RRScreenChangeNotifyMask);

		// event object
		XEvent ev;

		// variables to hold current screen state
		int width, height;
		Rotation rotation;

		// retrieve initial state
		width=XDisplayWidth(ptDisplay,iDefaultScreen);
		height=XDisplayHeight(ptDisplay,iDefaultScreen);
		XRRRotations(ptDisplay,iDefaultScreen, &rotation);

		int x11_fd=ConnectionNumber(ptDisplay);
		fd_set in_fds;
		FD_ZERO(&in_fds);
		FD_SET(x11_fd,&in_fds);
		while (true)
		{
			while (XPending(ptDisplay))
			{
				XNextEvent(ptDisplay,&ev);

				// if it's a notify event
				if (ev.type==event_base + RRScreenChangeNotify)
				{
					XRRScreenChangeNotifyEvent*chg=reinterpret_cast<XRRScreenChangeNotifyEvent*>(&ev);
					// If the values are different
					if (chg->width!=width || chg->height!=height || chg->rotation!=rotation)
					{
						width=chg->width;
						height=chg->height;
						rotation=chg->rotation;
						std::cout << "Dimensions: " << width << "x" << height << " at " << rotation << std::endl;
						// execute the user tool
						if (fork() == 0)
						{
							execvp(ptToolArgs[0],ptToolArgs+1);
							_exit(1);
						}
					}
					// Pass the event on to Xlib
					XRRUpdateConfiguration(&ev);
				}
			}
			// wait for an event
			select(x11_fd+1,&in_fds,0,0,NULL);
		}

	}
	catch (std::exception&e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	
	
	return 0;
}
