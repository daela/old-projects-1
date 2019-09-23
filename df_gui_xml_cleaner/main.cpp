//
// written lazily by nacitar sevaht
//
// use as you wish for whatever purpose, PUBLIC DOMAIN
//
// intended to:
// 	- reduce load time
//	- lessen occurances of the load screen/GUI bug
//	- keep your bank bags from arbitrarily losing their positions (because AV only lets the file store so much, things get discarded)
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <stack>
#include <set>
#include <stdexcept>

namespace nx
{
	// this is a very, very lazily written, to the point, no real error checking to speak of XML parser specifically intended for darkfall's xml files.
	// if you do something silly hand-editing the xml files, don't expect this to work for you.  It's pretty well written, tbh, but nothing amazing either.
	// I only implemented the functions I needed below, as well.
	struct xml_node
	{
		typedef std::list<std::pair<std::string,std::string> > value_list;
		typedef std::list<std::pair<std::string,xml_node> > child_list;
		value_list lstValues;
		child_list lstChildren;

		// Take "tag.subtag.subsubtag" and try to traverse it and give back a node
		xml_node* get_child_node(const std::string&strFullTag)
		{
			std::string::size_type uStart,uEnd;
			xml_node*ptNode=this;
			std::string strTag;
			child_list::iterator it;
			uStart=uEnd=0;
			// while we still have tokens
			while (uEnd != std::string::npos)
			{
				uEnd=strFullTag.find('.',uStart);
				// try to find the token in the tree
				it=ptNode->find(strFullTag.substr(uStart,uEnd-uStart));
				// if it's not there, bail out
				if (it == ptNode->end()) return NULL;
				// otherwise, continue until we've gotten the last token
				ptNode=&(it->second);
				
				uStart=uEnd+1;
			}
			// return our node.
			return ptNode;
		}
		void erase(child_list::iterator it)
		{
			lstChildren.erase(it);
		}	
		void clear()
		{
			lstChildren.clear();
		}
		
		std::string get(const std::string&strVar)
		{
			for (value_list::iterator it=lstValues.begin();it!=lstValues.end();++it)
			{
				if (it->first == strVar)
				{
					return it->second;
				}
			}
			return "";
		}
			
		void set(const std::string&strVar,const std::string&strValue)
		{
			for (value_list::iterator it=lstValues.begin();it!=lstValues.end();++it)
			{
				if (it->first == strVar)
				{
					it->second=strValue;
					return;
				}
			}
			lstValues.push_back(value_list::value_type(strVar,strValue));
		}
		void add_child(const std::string&strTag,const xml_node&obj)
		{
			lstChildren.push_back(child_list::value_type(strTag,obj));
		}
		child_list::iterator find(const std::string&strTag)
		{
			for (child_list::iterator it=lstChildren.begin();it!=lstChildren.end();++it)
			{
				if (it->first == strTag) return it;
			}
			return lstChildren.end();
		}
		child_list::iterator begin() { return lstChildren.begin(); }
		child_list::iterator end() { return lstChildren.end(); }
		
		// read darkfall's xml file.
		bool read(const std::string&strFilename)
		{
			char ch;
			std::ifstream ifs(strFilename.c_str(),std::ios::binary);
			if (!ifs) return false;
			enum XML_STATE { NOTHING, IN_TAG, IN_FIELD, IN_DATA, TAG_DONE };
			XML_STATE uState=NOTHING;
			std::string strTag;
			std::string strField, strValue;
			bool isClose;
			bool isLeaf;
			bool isQuoted;
			bool isTagRead;
			std::stack<xml_node*> stTree;
			stTree.push(this);
			// this is modelled as a state machine
			// NOTHING, base state, nothing to do.
			// IN_TAG, we found a < and now are looking for the tag's name
			// IN_DATA, we found the name and are now looking for values like Value="5" Text="wee", etc..
			// TAG_DONE a state to tell the code following the state machine to insert the node into the tree.
			while (ifs.get(ch))
			{
				switch (uState)
				{
					case NOTHING:
						switch (ch)
						{
							case ' ':case '\t':case '\r':case '\n':
								break;
							case '<':
								uState=IN_TAG;
								isClose=false;
								isLeaf=false;
								isTagRead=false;
								strTag.clear();
								break;
							default:
								throw std::runtime_error("Invalid input, got text outside of a tag, which is not supported.  All values but be properties of a tag.");
						}
						break;
					case IN_TAG:
						if (strTag.empty() && ch == '/')
							isClose=true;
						else
						{
							// anything but default results in fully read
							isTagRead=true;
							switch (ch)
							{
								case '/': isLeaf=true; break;

								case ' ':case '\t':case '\r':case '\n':
									uState=IN_FIELD; strField.clear(); break;

								case '>': uState=TAG_DONE; break;
								default:
									isTagRead=false;
									if (ch != '?' && !std::isalnum(ch) && ch != '_') { std::cout << "WTF " << ch << std::endl; throw std::runtime_error("Invalid character in tag name."); }
									strTag += ch;
							}
						}
						break;
					case IN_FIELD:
						if (strField.empty() && (ch == ' ' || ch == '\t')) break;
						switch (ch)
						{
							case '=':
								uState = IN_DATA; strValue.clear(); isQuoted=false; break;
							case '?':
							case '/': isLeaf=true; break;
							case '>': uState=TAG_DONE; break;
							default:
								if (!std::isalnum(ch) && ch != '_') { throw std::runtime_error("Invalid character in field name."); }
								strField += ch;
						}
						break;
					case IN_DATA:
						if (!isQuoted)
						{
							if (ch != '"') throw 1;
							isQuoted=true;
						}
						else if (ch == '"')
						{
							// add the field
							stTree.top()->lstValues.push_back(value_list::value_type(strField,strValue));
							uState=IN_FIELD;
							strField.clear();
						}
						else
							strValue += ch;
						break;
					case TAG_DONE:
						throw std::runtime_error("Something straight fucked up, you shouldn't see this."); // shouldnt see this value here, it's unset below. 
				}
				// if there's a tag name read and ready to be added, that isn't a closing tag </bla>, add it to the tree and put it on top.
				if (isTagRead && !isClose)
				{
					isTagRead=false;
					stTree.top()->lstChildren.push_back(child_list::value_type(strTag,xml_node()));
					stTree.push( &(stTree.top()->lstChildren.back().second));
				}
				// if we're done reading the tag's attributes, remove it from the tree if we can determine that there aren't going to be any children.
				if (uState == TAG_DONE)
				{
					if (isLeaf || isClose || strTag == "?xml") stTree.pop();
					uState=NOTHING;
				}
			}
			return true;
		}

		//std::string strValue;
		friend std::ostream&operator<<(std::ostream&os,const xml_node&obj);
	};
	std::ostream&operator<<(std::ostream&os,const xml_node&obj)
	{
		typedef std::pair<const xml_node*,xml_node::child_list::const_iterator> node_iterator_pair;
		bool isDeeper=false;
		// so it is always \r\n, not just \n on linux.. so you can filter your xml files on linux, too.
		const std::string win_endl="\r\n";

		// stTree holds the current node as well as the iterator to it, so that when we output a child, and its children, we can resume enumeration where we left off.
		std::stack<node_iterator_pair> stTree;
		// set it up to start
		stTree.push(node_iterator_pair(&obj,obj.lstChildren.begin()));
		while (!stTree.empty())
		{
			const std::string strIndent((stTree.size()-1)*2,' ');
			
			for (xml_node::child_list::const_iterator it=stTree.top().second;it!=stTree.top().first->lstChildren.end();++it)
			{
				// start outputting the tag
				os << strIndent << "<" << it->first;
				// output its attributes
				for (xml_node::value_list::const_iterator vit=it->second.lstValues.begin();vit!=it->second.lstValues.end();++vit)
				{
					os << " " << vit->first << "=\"" << vit->second << "\"";
				}
				// if it was a tag like the ?xml tag, close it with ?>
				if (!it->first.empty() && it->first[0] == '?')
				{
					os << "?>" << win_endl;
				}
				else
				{
					// if it has children, we must save our state on the stack and push our child node to the top, and terminate the tag with >
					if (!it->second.lstChildren.empty())
					{
						xml_node::child_list::const_iterator tit=it;
						stTree.top().second = it;
						isDeeper=true;
						os << ">" << win_endl;
						stTree.push(node_iterator_pair(&it->second,it->second.lstChildren.begin()));
						
						break;
					}
					// if it has no children, then we can just express it this way and not push it to the stack
					os << "/>" << win_endl; // no children
				}
			}
			// if we get here and it wasn't because we broke out of the above loop to process a child node, then we're done with our current node, and can remove it from the stack.
			if (!isDeeper)
			{
				stTree.pop();
				// this condition should never not be true, but just to be safe.
				if (!stTree.empty())
				{
					const std::string strIndent((stTree.size()-1)*2,' ');
					xml_node::child_list::const_iterator &tit=stTree.top().second;
					os << strIndent << "</" << tit->first << ">" << win_endl;
					++tit;
				}
			}
			isDeeper=false;
		}
		return os;
	}
}

// driver code is pretty clear, I delete from
// - GuiPersistence.Chat.WIndowN.Editor
// - GuiPersistence.Chat.History
// - GuiPersistence.Chat.ViewN
// - GuiPersistence.WindowFactories_Dynamic.EntryN

int main(int argc,char*argv[])
{	
	nx::xml_node objXML;
	typedef nx::xml_node::child_list child_list;
	std::string strFilename, strOutputFilename;
	if (argc == 3)
	{
		strFilename = argv[1];
		strOutputFilename = argv[2];
	}
	else
	{
		std::cout << "Usage: " << argv[0] << " <input gui_persistence filename> <output gui_persistence filename>" << std::endl;
		return 1;
	}
	bool bSuccess=false;
	try
	{
		bSuccess=objXML.read(strFilename.c_str());
	}
	catch (std::exception&e)
	{
		bSuccess=false;
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 2;
	}
	if (bSuccess)
	{
		nx::xml_node*ptNode=objXML.get_child_node("GuiPersistence.Chat");
		if (ptNode)
		{
			unsigned long uViewCount=0,uDeletedViewCount=0,uSelfChatsCleared=0;
			for (child_list::iterator it=ptNode->begin();it!=ptNode->end();)
			{
				
				if (it->first.substr(0,6) == "Window" && it->first != "WindowCount")
				{
					nx::xml_node *ptEditor=it->second.get_child_node("Editor");
					if (ptEditor)
					{
						nx::xml_node*ptLength=ptEditor->get_child_node("Length");
						if (!ptLength || ptLength->get("Value")!="0")
						{
							++uSelfChatsCleared;
							ptEditor->clear();
							nx::xml_node objLength;
							objLength.set("Value","0");
							ptEditor->add_child("Length",objLength);
							//std::cout << "Deleted self chat history for " << it->first << std::endl;
						}
					}
				}
				else if (it->first.substr(0,4) == "View" && it->first != "ViewCount")
				{
					nx::xml_node*ptWinID=it->second.get_child_node("WindowID");
					nx::xml_node*ptDefaultCmd=it->second.get_child_node("DefaultCommand");
					//nx::xml_node*ptDisplayName=it->second.get_child_node("DisplayName");
					if (ptWinID && ptDefaultCmd && ptWinID->get("Value") == "chat" && ptDefaultCmd->get("Value").substr(0,4) == "msg ")
					{
					
						child_list::iterator tmp=it;
						++it;
						//std::cout << "Deleted /tell entry for " << ptDisplayName->get("Value") << std::endl;
						ptNode->erase(tmp);
						++uDeletedViewCount;
						continue;
					}
					std::stringstream ss;
					ss << "View" << uViewCount;
					it->first = ss.str();
					++uViewCount;
				}
				
				++it;
			}
			if (uSelfChatsCleared)
				std::cout << "Deleted " << uSelfChatsCleared << " window input history caches." << std::endl;

			if (uDeletedViewCount)
				std::cout << "Deleted " << uDeletedViewCount << " /tell entries." << std::endl;

			nx::xml_node*ptViewCount=ptNode->get_child_node("ViewCount");
			if (ptViewCount)
			{
				std::stringstream ss;
				ss << uViewCount;
				ptViewCount->set("Value",ss.str());
			}
			nx::xml_node*ptHistory=ptNode->get_child_node("History");
			if (ptHistory)
			{
				nx::xml_node*ptEntryCount=ptHistory->get_child_node("EntryCount");
				if (!ptEntryCount || ptEntryCount->get("Value")!="0")
				{
					ptHistory->clear();
					nx::xml_node objEntryCount;
					objEntryCount.set("Value","0");
					ptHistory->add_child("EntryCount",objEntryCount);
					std::cout << "Deleted ALL chat logs." << std::endl;
				}
			}
				
		}
		ptNode=objXML.get_child_node("GuiPersistence.WindowFactories_Dynamic");
		std::set<std::string> stUnique;
		if (ptNode)
		{
			enum EntryType { ENTRY_BANK, ENTRY_VAULT, ENTRY_TRADE, ENTRY_CHARACTER, ENTRY_UNKNOWN, ENTRY_COUNT=ENTRY_UNKNOWN };
			unsigned long uEntryCount=0;
			unsigned long uDeletedEntries[ENTRY_COUNT] = { 0 };
			for (child_list::iterator it=ptNode->begin();it!=ptNode->end();)
			{
				
				if (it->first.substr(0,5) == "Entry")
				{
					nx::xml_node*ptNameOfRoot=it->second.get_child_node("NameOfRoot");
					EntryType uEntry=ENTRY_UNKNOWN;
					if (ptNameOfRoot)
					{
						std::string strValue=ptNameOfRoot->get("Value");
						if (strValue=="Clan Vault Root")
							uEntry=ENTRY_VAULT;
						else if (strValue=="Trading_Own" || strValue=="Trading_Other")
							uEntry=ENTRY_TRADE;
						else if (strValue=="Character")
							uEntry=ENTRY_CHARACTER;
						else if (strValue=="Bank")
							uEntry=ENTRY_BANK;
						else
						{
							std::cout << "!!! UNKNOWN ITEM ENTRY TYPE: " << strValue << std::endl;
						}
					}
						
					if (uEntry != ENTRY_BANK && uEntry != ENTRY_UNKNOWN)
					{
						
						child_list::iterator tmp=it;
						++it;
						//std::cout << "Deleted clan bank entry." << std::endl;
						ptNode->erase(tmp);
						++uDeletedEntries[uEntry];
						continue;
					}
					if (ptNameOfRoot && uEntry==ENTRY_UNKNOWN)
					{
						if (stUnique.insert(ptNameOfRoot->get("Value")).second)
							std::cout << ptNameOfRoot->get("Value") << std::endl;
					}

					std::stringstream ss;
					ss << "Entry" << uEntryCount;
					it->first = ss.str();
					++uEntryCount;
				}
				++it;
			}
			for (unsigned int i=0;i<ENTRY_COUNT;++i)
			{
				if (uDeletedEntries[i])
				{
					std::cout << "Deleted " << uDeletedEntries[i] << " ";
					switch (i)
					{
						case ENTRY_VAULT: std::cout << "clan vault"; break;
						case ENTRY_TRADE: std::cout << "trade window"; break;
						case ENTRY_CHARACTER: std::cout << "character"; break;
						case ENTRY_BANK: std::cout << "bank"; break;
					}
					std::cout << " item entries." << std::endl;
				}
			}

			nx::xml_node*ptEntryCount=ptNode->get_child_node("Count");
			if (ptEntryCount)
			{
				std::stringstream ss;
				ss << uEntryCount;
				ptEntryCount->set("Value",ss.str());
			}
		}
	}
	else
	{
		std::cerr << "ERROR: Could not open " << strFilename << " for reading." << std::endl;
		return 3;
	}
	
	std::ofstream ofs(strOutputFilename.c_str(),std::ios::binary);
	if (!ofs)
	{
		std::cerr << "ERROR: Could not open " << strOutputFilename << "for writing." << std::endl;
		return 3;
	}
	ofs << objXML;
	std::cout << "Success!" << std::endl;
	return 0;
}
