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

  wstring mSyncerName;
  wstring mOutPath;
  wstring mContext;
  wstring mRole;
  wstring mCmdFile;
  wstring mConfigRootPath;

  bool GetBackroomServers();
  BackroomServerList *GetServerListOnSameSubnet(const string& server);

  bool GenerateConfigFile(const string& server);
};



#endif

