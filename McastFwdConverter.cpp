// McastFwdConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "McastFwdConverter.h"
#include <set>
using namespace std;

#define MCAST_TCP_PORT "7778"
#define MCAST_UDP_PORT "10779"

const wstring kTemplateDir = L"syncer\\McastFwd\\";
const wstring kEndDestination = L"_ConfigRoot\\McastFwd\\";

bool ReplaceSubString(string& s, const string&from, const string& to)
{
  size_t pos = s.find(from);
  if (pos != string::npos)
  {
    s.replace(pos, from.length(), to);
  }
  return false;
}

int _tmain(int argc, _TCHAR* argv[])
{

  cSyncerHelper::cBenchmark benchmark("cMcastFwdConverter");
  cSyncerHelper::Log("cMcastFwdConverter Starting with:");

  _TCHAR* pSyncerName = cSyncerHelper::GetCmdOption(argv, argv + argc, L"-name");
  _TCHAR* pOutPath = cSyncerHelper::GetCmdOption(argv, argv + argc, L"-out");
  _TCHAR* pContext = cSyncerHelper::GetCmdOption(argv, argv + argc, L"-context");
  _TCHAR* pRole = cSyncerHelper::GetCmdOption(argv, argv + argc, L"-role");
  _TCHAR* pCmdFile = cSyncerHelper::GetCmdOption(argv, argv + argc, L"-cmd");

  wstring syncerName, outPath, context, role, cmdFile, test;
  
  if (pSyncerName)
  {
    syncerName = pSyncerName;
    cSyncerHelper::Log(L"- syncer name: " + syncerName);
  }
  if (pOutPath)
  {
    outPath = pOutPath;
    cSyncerHelper::Log(L"- output file: " + outPath);
  }
  if (pContext)
  {
    context = pContext;
    cSyncerHelper::Log(L"- context: " + context);
  }
  if (pRole)
  {
    role = pRole;
    cSyncerHelper::Log(L"- role: " + role);
  }
  if (pCmdFile)
  {
    cmdFile = pCmdFile;
    cSyncerHelper::Log(L"- command file: " + cmdFile);
  }

  //if (syncerName.empty() || outPath.empty() || role.empty() || cmdFile.empty())
  //{
  //  cSyncerHelper::Log("Argument missing; abort");
  // // return -1;
  //}
  
  cMcastFwdConverter converter(syncerName, outPath, context, role, cmdFile);
  return converter.Run();
}


cMcastFwdConverter::cMcastFwdConverter(wstring syncerName, wstring outPath, wstring context, wstring role, wstring cmdFile)
  : mSyncerName(syncerName), mOutPath(outPath), mContext(context), mRole(role), mCmdFile(cmdFile)
{
  if (mOutPath.find_last_of(L"/\\") != mOutPath.length() - 1)
  {
    // make sure the path ended with '\'
    mOutPath += L"\\";
  }
}

cMcastFwdConverter::~cMcastFwdConverter()
{

}

int cMcastFwdConverter::Run()
{
  mConfigRootPath = GetConfigRootEnvVar();
  if (mConfigRootPath.empty())
  {
    Log("Cannot find configroot environment variable; abort");
    return -1;
  }
  else if (mConfigRootPath.find_last_of(L"/\\") != mConfigRootPath.length() - 1)
  {
    // make sure the path ended with '\'
    mConfigRootPath += L"\\";
  }

  if (!Connect())
  {
    Log("Cannot connect to database; abort");
    return -1;
  }

  if (!GetViperSystem())
  {
    Log("Cannot get vipersystem configuration; abort");
    return -1;
  }

  if (!GetBackroomServers())
  {
    Log("Cannot get backroomserver configuration; abort");
    return -1;
  }

  string serverName = GetServerNameByContext(mContext);
  if (!serverName.empty())
  {
    GenerateConfigFile(serverName);
    GenerateConfigFile("");   // for satellite sites
  }

  return 0;
}

bool cMcastFwdConverter::GetViperSystem()
{
  const string query = "SELECT * FROM vipersystem";

  cSelfDeletingResultSet resultSet;
  resultSet.mpRes = ExecuteQuery(query);

  if (resultSet.mpRes)
  {
    while (resultSet.mpRes->next())
    {
      mViperSystem.domainName = ResultGetStringDef(resultSet.mpRes, "domainName");
      mViperSystem.dnsFwder1 = ResultGetStringDef(resultSet.mpRes, "dnsFwder1");
      mViperSystem.dnsFwder2 = ResultGetStringDef(resultSet.mpRes, "dnsFwder2");
      mViperSystem.dnsFwder3 = ResultGetStringDef(resultSet.mpRes, "dnsFwder3");
      mViperSystem.dnsAllowUpdate1 = ResultGetStringDef(resultSet.mpRes, "dnsAllowUpdate1");
      mViperSystem.dnsAllowUpdate2 = ResultGetStringDef(resultSet.mpRes, "dnsAllowUpdate2");
      mViperSystem.dnsAltFwderEnable = ResultGetStringDef(resultSet.mpRes, "dnsAltFwderEnable");
      mViperSystem.dnsAltDomain = ResultGetStringDef(resultSet.mpRes, "dnsAltDomain");
      mViperSystem.dnsAltFwder = ResultGetStringDef(resultSet.mpRes, "dnsAltFwder");
      mViperSystem.dhcpDefaultEnable = (ResultGetStringDef(resultSet.mpRes, "dhcpDefaultEnable") == "1");
      mViperSystem.dhcpNextServer = ResultGetStringDef(resultSet.mpRes, "dhcpNextServer");
      mViperSystem.dhcpTftpServer = ResultGetStringDef(resultSet.mpRes, "dhcpTftpServer");
      mViperSystem.dhcpFileName = ResultGetStringDef(resultSet.mpRes, "dhcpFileName");
    }
  }

  return true;
}

bool cMcastFwdConverter::GetBackroomServers()
{
  const string query = "SELECT a.name, a.ipAddress, b.ViperSubnet_id \
                       FROM backroomserver a, vipernode b WHERE a.type = \"BackRoom\" and a.ViperNode_id = b.id \
                       ORDER BY b.ViperSubnet_id, a.name;";

  cSelfDeletingResultSet resultSet;
  resultSet.mpRes = ExecuteQuery(query);
  if (!resultSet.mpRes)
  {
    return false;
  }

  string lastSubnetId = "-1";
  mBackroomServerOnSubnet.clear();
  BackroomServerList *pBackroomServerList = 0;
  while (resultSet.mpRes->next())
  {
    BackroomServer backroomServer;
    backroomServer.name = ResultGetStringDef(resultSet.mpRes, "name");
    backroomServer.ipAddress = ResultGetStringDef(resultSet.mpRes, "ipAddress");
    backroomServer.subnetId = ResultGetStringDef(resultSet.mpRes, "ViperSubnet_id");


    if (lastSubnetId != backroomServer.subnetId)
    {
      pBackroomServerList = new BackroomServerList;
      mBackroomServerOnSubnet.push_back(pBackroomServerList);

      lastSubnetId = backroomServer.subnetId;
    }

    if (pBackroomServerList)
    {
      pBackroomServerList->push_back(backroomServer);
    }

  }

  string serverList = "found backroom servers on each subnet: \n";
  for (unsigned int i = 0; i < mBackroomServerOnSubnet.size(); i++)
  {
    serverList += "[" + ToStr(i) + "] ";
    for (unsigned int j = 0; j < mBackroomServerOnSubnet[i]->size(); j++)
    {
      serverList += (*mBackroomServerOnSubnet[i])[j].name + " ";
    }
    serverList += "\n";
  }
  Log("GetBackroomServers() " + serverList);

  return true;
}

cMcastFwdConverter::BackroomServerList *cMcastFwdConverter::GetServerListOnSameSubnet(const string& server)
{
  for (unsigned int i = 0; i < mBackroomServerOnSubnet.size(); i++)
  {
    for (unsigned int j = 0; j < mBackroomServerOnSubnet[i]->size(); j++)
    {
      if (server == (*mBackroomServerOnSubnet[i])[j].name)
      {
        return mBackroomServerOnSubnet[i];
      }
    }
  }

  return 0;
}

bool cMcastFwdConverter::GenerateConfigFile(const string& server)
{
  Log("cMcastFwdConverter::GenerateConfigFile() for server: " + server);

  wstring finalPath = mOutPath + kEndDestination;
  if (server.empty())
  {
    finalPath = mConfigRootPath + L"\\McastFwd\\";
  }
  if (!DirectoryExists(finalPath))
  {
    CreateDirectoryRecursively(finalPath);
  }

  wstring templateFileName = mConfigRootPath + kTemplateDir + L"McastFwd.xml.template";
  wstring cfgFileName = finalPath + L"McastFwd.xml";
  wstring tmpCfgFileName = cfgFileName + L".tmp";

  cXMLDoc *pXmlDoc = CreateXMLFromTemplate(templateFileName, cfgFileName);
  if (!pXmlDoc)
  {
    Log(L"GenerateConfigFile: CreateXMLFromTemplate() failed: template = " + templateFileName + L", filename = " + cfgFileName);
    return false;
  }

  DOMNode *pRootNode = pXmlDoc->GetRootNode();
  if (pRootNode)
  {
    // mode
    DOMNode *pModeNode = NULL;
    string mode = "Client"; 
    if (!server.empty())
    {
      if (mBackroomServerOnSubnet.size() > 0
        && GetServerListOnSameSubnet(server) == mBackroomServerOnSubnet[0])
      {
        mode = "Server";
      }
      else
      {
        mode = "Server+Client";
      }
    }
    pXmlDoc->CreateChildTextNode("Mode", mode, pRootNode, &pModeNode);
    
    // fwdList
    DOMNode *pFwdList = NULL;
    pXmlDoc->CreateChildNode("FwdList", pRootNode, &pFwdList);
    if (pFwdList)
    {
      for (unsigned int i = 0; i < mBackroomServerOnSubnet.size(); i++)
      {
        if (i != 0 && !server.empty() && GetServerListOnSameSubnet(server) == mBackroomServerOnSubnet[i])
        {
          break;
        }

        DOMNode *pFwdGrp = NULL;
        pXmlDoc->CreateChildNode("FwdGrp", pFwdList, &pFwdGrp, false);
        if (pFwdGrp)
        {
          pXmlDoc->SetNodeAttribute(pFwdGrp, "id", ToStr(i + 1));
          pXmlDoc->SetNodeAttribute(pFwdGrp, "name", "PSAP" + ToStr(i + 1));
          pXmlDoc->SetNodeAttribute(pFwdGrp, "ignoreIfNotInDns", (i == 0) ? "false" : "true");

          for (unsigned int j = 0; j < mBackroomServerOnSubnet[i]->size(); j++)
          {
            DOMNode *pConn = NULL;
            pXmlDoc->CreateChildNode("Connection", pFwdGrp, &pConn, false);
            if (pConn)
            {
              pXmlDoc->SetNodeAttribute(pConn, "server", (*mBackroomServerOnSubnet[i])[j].name);
              pXmlDoc->SetNodeAttribute(pConn, "port", MCAST_TCP_PORT);
              pXmlDoc->SetNodeAttribute(pConn, "udpport", MCAST_UDP_PORT);
            }
          }
        } //if (pFwdGrp)
      }
    } //if (pFwdList)
    
    // NodeManagerClient
    DOMNode *pNodeMgrClientNode = NULL;
    string value = (!server.empty() ? "true" : "false");
    pXmlDoc->CreateChildTextNode("NodeManagerClient", value, pRootNode, &pNodeMgrClientNode);

    // Comment
    DOMNode *pCommentNode = NULL;
    string comment = "This configuration is generated for " + (!server.empty() ? server : "satellite site");
    pXmlDoc->CreateChildTextNode("Comment", comment, pRootNode, &pCommentNode);
  }

  WriteXMLFile(pXmlDoc);

  bool rc = true;
  try
  {
    if (FileCopyIfDifferent(tmpCfgFileName, cfgFileName))
    {
      Log(L"File updated: " + cfgFileName);
      // restart process to take the new config file
      if (!AppendToFile(mCmdFile, kEndDestination + cfgFileName, true, true))
      {
        Log(L"Unable to write to command file " + mCmdFile);
      }
    }
    else
    {
      Log(L"File is the same: " + finalPath + cfgFileName);
    }
  }
  catch (std::exception e)
  {
    Log("Failure: " + string(e.what()));
    FileDelete(tmpCfgFileName);
    return false;
  }

  FileDelete(tmpCfgFileName);
  return true;
}

