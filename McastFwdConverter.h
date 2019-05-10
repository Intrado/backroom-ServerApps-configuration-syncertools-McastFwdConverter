#ifndef MCASTFWDCONVERTER_H
#define MCASTFWDCONVERTER_H

#include "SyncerHelper.h"
#include "XmlHelper.h"

class cMcastFwdConverter : public cSyncerHelper, public cXmlHelper
{
public:
  cMcastFwdConverter(wstring syncerName, wstring outPath, wstring context, wstring role, wstring cmdFile);
  virtual ~cMcastFwdConverter();

  virtual int Run();

private:
  struct BackroomServer {
    string name;
    string ipAddress;
    string subnetId;
    inline bool operator < (const BackroomServer& x) { return name.compare(x.name) < 0; };
  };
  typedef vector<BackroomServer> BackroomServerList;
  vector<BackroomServerList *> mBackroomServerOnSubnet;

  struct ViperSystem {
    string domainName;          // vipersystem.domainName
    string dnsFwder1;           // vipersystem.dnsFwder1
    string dnsFwder2;           // vipersystem.dnsFwder2
    string dnsFwder3;           // vipersystem.dnsFwder3
    string dnsAllowUpdate1;     // vipersystem.dnsAllowUpdate1
    string dnsAllowUpdate2;     // vipersystem.dnsAllowUpdate2
    string dnsAltFwderEnable;   // vipersystem.dnsAltFwderEnable
    string dnsAltDomain;        // vipersystem.dnsAltDomain
    string dnsAltFwder;         // vipersystem.dnsAltFwder
    bool   dhcpDefaultEnable;   // vipersystem.dhcpDefaultEnalbe
    string dhcpNextServer;      // vipersystem.dhcpNextServer
    string dhcpTftpServer;      // vipersystem.dhcpTftpServer
    string dhcpFileName;        // vipersystem.dhcpFileName
    string mViperSubnets;       // from vipernode, vipersubnet eg: 10.103.40.0/24;10.5.0.0/20;10.8.5.0/24
  };
  ViperSystem mViperSystem;

  wstring mSyncerName;
  wstring mOutPath;
  wstring mContext;
  wstring mRole;
  wstring mCmdFile;
  wstring mConfigRootPath;

  bool GetViperSystem();
  bool GetBackroomServers();
  BackroomServerList *GetServerListOnSameSubnet(const string& server);

  bool GenerateConfigFile(const string& server);
};



#endif

