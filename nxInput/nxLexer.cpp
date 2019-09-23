#include "nxLexer.hpp"

const nxDFA_Node* nxDFA_Node::Input(char ch) const
{
    std::map<char,nxDFA_Node>::const_iterator it = mpInput.find(ch);
    if (it != mpInput.end())
        return &(it->second);
    return NULL;
}

nxDFA_Node* nxDFA_Node::Add(char ch,unsigned long id)
{
    std::map<char,nxDFA_Node>::iterator it = mpInput.find(ch);
    if (it != mpInput.end())
    {
        // If there's already an identifier here, return NULL
        if (id != 0)
        {
            if (it->second.uId != 0)
                return NULL;
            it->second.uId = id;
        }
    }
    else
        it=mpInput.insert(std::map<char,nxDFA_Node>::value_type(ch,nxDFA_Node(id))).first;

    return &(it->second);
}

bool nxDFA::AddLexeme(const std::string&strLexeme,const unsigned long&id)
{
    if (!id || strLexeme.empty()) return false;
    nxDFA_Node*ptNode = this;
    std::string::const_iterator it=strLexeme.begin();
    while (ptNode != NULL)
    {
        char ch=*it++;
        if (it == strLexeme.end())
        {
            ptNode = ptNode->Add(ch,id);
            break;
        }
        else
            ptNode = ptNode->Add(ch);
    }
    return (ptNode != NULL);
}





bool nxLexer::AddIdentifier(const std::string&strIdentifier,const unsigned long&uId)
{
    for (std::string::const_iterator it = strIdentifier.begin();it!=strIdentifier.end();++it)
    {
        if (!IsIdentifierChar(*it))
            return false;
    }
    if (strIdentifier.empty() || IsNum(strIdentifier[0]))
        return false;
    return dfaIdentifiers.AddLexeme(strIdentifier,uId);
}
bool nxLexer::AddOperator(const std::string&strOperator,const unsigned long&uId)
{
    for (std::string::const_iterator it = strOperator.begin();it!=strOperator.end();++it)
    {
        if (!IsOperatorChar(*it))
            return false;
    }
    if (strOperator.empty() || IsNum(strOperator[0]))
        return false;
    return dfaOperators.AddLexeme(strOperator,uId);
}
nxLexer::nxLexer(const std::string&strIdentifierCharset,const std::string&strOperatorCharset)
{
    for (std::string::const_iterator it=strIdentifierCharset.begin();it!=strIdentifierCharset.end();++it)
        stIdentifierCharset.insert(*it);
    for (std::string::const_iterator it=strOperatorCharset.begin();it!=strOperatorCharset.end();++it)
        stOperatorCharset.insert(*it);
}
