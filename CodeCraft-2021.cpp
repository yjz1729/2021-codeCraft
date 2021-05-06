#include <set>
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <map>
#include <utility>
#include <cmath>
#include <exception>
#include <deque>
using namespace std;

#define GETSERVER(index) (index>=serverVec.size()?helpServerVec[index-serverVec.size()]:serverVec[index])

constexpr double divConst = 1.5;
constexpr double weightConst_1 = 1.5;
constexpr double weightConst_2 = 3;
constexpr double cutConst = 0.3;
constexpr double overlapRatioConst = 0;
constexpr double overlapRatioConstAdd = 7;
constexpr double emptyConst = 0.9;
constexpr int maxMigConst = 200;

int classNumConst = 0;
struct MigrationOutputType;
struct QuestOutputType;
struct ServerType;
class ServerList;
struct VirtualType;
class VirtualList;
struct QuestType;
class QuestList;
class VirtualNode;
class VirtualHeap;
class SingleServerNode;
class ServerNode;

int static_days = -1;

std::vector<MigrationOutputType> migrationOutputVec;


//====================myerror.h=========================
class MyError : public std::exception
{
private:
	std::string mMsg, mPlace;
public:
	MyError(const char* msg) :mMsg(msg) {}
	MyError(const std::string& msg) :mMsg(msg) {}
	MyError(const char* msg, const char* place) :mMsg(msg), mPlace(place) {}
	MyError(const std::string& msg, const std::string& place) :mMsg(msg), mPlace(place) {}
	virtual const char* what()const noexcept override { return mMsg.c_str(); }
	virtual const char* where()const noexcept { return mPlace.c_str(); }
};


//====================typeandlist.h=========================
struct MigrationOutputType
{
	int virtualId, serverId, inA;
	MigrationOutputType(int mVirtualId, int mServerId, int mInA) : virtualId(mVirtualId), serverId(mServerId), inA(mInA) {};
};

struct ServerType
{
	double weight(const int& coreNum, const double& weightConst)const;
	std::string type;
	int coreNum, memNum, priceCost, powerCost;
	int classFlag = 0;  //which class of server is.
	void input();
	double cutCore(const double& ratio) const;
	double getPrice(const int& days, const double& weightConst, const double& ratio) const;
};

class ServerList
{
private:
	std::vector<ServerType> serverVec;
	std::vector<std::vector<int>> serverVecDays;
public:
	void setDays(const int& days); //all days
	void input();
	const ServerType& getServer(const int& classFlag, const int& dayNum) { return serverVec[serverVecDays[classFlag][dayNum]]; };
}serverList;


struct VirtualType
{
	std::string type;
	int coreNum, memNum;
	int classFlag = 0;  //which class of server is.
	int classFlagAdd = 0;  //which class of server is.
	bool duo;
	void input();
	friend bool operator< (const VirtualType& a, const VirtualType& b); //cmp ratio
};

class VirtualList
{
private:
	std::unordered_map<std::string, VirtualType>virtualListMap; //search frequently so map 
public:
	void input();
	void setClass(std::vector<double>& ratioVec, std::vector<int>& maxCoreVec, std::vector<int>& maxMemVec, std::vector<int>& middleCoreVec);
	const VirtualType& operator [](const std::string& type) { return virtualListMap[type]; };
}virtualList;

struct QuestType
{
	bool addOrDel; //add:1  del:0
	std::string type;
	int virtualId;
	int serverId = -1, inA = -1;

	void input();
};

class QuestList
{
private:
	std::vector<std::vector<QuestType>> questVec;
	int nowDay = 0;
public:
	int T = -1, K = -1;
	void set();
	void input();
	void output(const int& day);
	void input(const int& days);
	std::vector<QuestType>& operator [](const int& dayNum) { return questVec[dayNum]; };
}questList;


//============server type===============
void ServerType::input()
{
	char c;
	std::cin >> c >> type >> coreNum >> c >> memNum >> c >> priceCost >> c >> powerCost >> c;
	type.pop_back();
}

double ServerType::weight(const int& coreNum, const double& weightConst) const
{
	if (coreNum < weightConst_1 * weightConst)
		return exp(-coreNum / (weightConst)+weightConst_1);
	else
		return exp(coreNum / (weightConst_1 * weightConst_2 * weightConst) - 1.0 / weightConst_2);
}

double ServerType::getPrice(const int& days, const double& weightConst, const double& ratio) const
{
	return (priceCost + static_cast<double>(days) * powerCost) / cutCore(ratio) * weight(coreNum, weightConst);
}

double ServerType::cutCore(const double& ratio) const
{
	if (double(coreNum) / memNum >= ratio)
		return memNum * ratio + log(cutConst * (coreNum - memNum * ratio) + 1);
	else
		return coreNum + log(cutConst * (memNum - coreNum / ratio) + 1);
}

//============server list===============
void ServerList::input()
{
	int n;
	std::cin >> n;
	serverVec.resize(n);
	for (int i = 0; i < n; i++)
		serverVec[i].input();
}
void ServerList::setDays(const int& days)
{
	std::vector<double> ratioVec;
	std::vector<int> maxCoreVec;
	std::vector<int> maxMemVec;
	std::vector<int> middleCoreVec;


	virtualList.setClass(ratioVec, maxCoreVec, maxMemVec, middleCoreVec);


	serverVecDays.resize(classNumConst);

	int veclen = serverVec.size();
	int j, k;
	double price, nowprice;
	int serverTypeId;
	for (int i = 0; i < classNumConst; i++)
	{
		for (j = 0; j < days; j++)
		{
			price = 1e20;
			serverTypeId = -1;
			for (k = 0; k < veclen; k++)
				if (serverVec[k].coreNum >= maxCoreVec[i] && serverVec[k].memNum >= maxMemVec[i])
					if (price > (nowprice = serverVec[k].getPrice(days - j, maxCoreVec[i], ratioVec[i])))
					{
						price = nowprice;
						serverTypeId = k;
					}
			if (serverTypeId == -1)
				throw MyError("no server is fit", "ServerList::setDays");
			serverVecDays[i].push_back(serverTypeId);
			serverVec[serverTypeId].classFlag |= (1 << i);
		}
	}
}

//============virtual type===============
void VirtualType::input()
{
	char c;
	std::cin >> c >> type >> coreNum >> c >> memNum >> c >> duo >> c;
	type.pop_back();
}

bool operator< (const VirtualType& a, const VirtualType& b)
{
	return double(a.coreNum) / a.memNum < double(b.coreNum) / b.memNum;
}

//============virtual list===============
void VirtualList::input()
{
	int m;
	std::cin >> m;
	VirtualType help;
	for (int i = 0; i < m; i++)
	{
		help.input();
		virtualListMap[help.type] = help;
	}
}

int PartSort(std::vector<int>& coreNum, int start, int end)
{
	int left = start;
	int right = end;
	int key = coreNum[end];   //选取关键字
	while (left < right)
	{
		while (left < right && coreNum[left] <= key)  //左边找比key大的值
		{
			++left;
		}
		while (left < right && coreNum[right] >= key)  //右边找比key小的值
		{
			--right;
		}
		if (left < right)
		{
			swap(coreNum[left], coreNum[right]);  //找到之后交换左右的值
		}
	}
	swap(coreNum[right], coreNum[end]);
	return left;
}


void VirtualList::setClass(std::vector<double>& ratioVec, std::vector<int>& maxCoreVec, std::vector<int>& maxMemVec, std::vector<int>& middleCoreVec)
{


	std::vector<VirtualType> helpVirtualList;
	std::vector<int> helpCoreNum;

	std::vector<int> helpCoreNumChange;

	std::vector<double> coreMemRatios;

	for (const auto& e : virtualListMap)
	{
		helpVirtualList.push_back(e.second);

		helpCoreNum.push_back((e.second.duo ? e.second.coreNum : e.second.coreNum * 2));

		coreMemRatios.push_back(double(e.second.coreNum) / e.second.memNum);
	}

	int veclen = helpVirtualList.size();
	int* ptr = static_cast<int*>(malloc(sizeof(int) * veclen));
	for (int i = 0; i < veclen; i++)
		ptr[i] = i;
	sort(ptr, ptr + veclen, [helpVirtualList](const int& a, const int& b) {return helpVirtualList[a] < helpVirtualList[b]; });
	sort(coreMemRatios.begin(), coreMemRatios.end());
	int start, end;
	int allCore, allMem;
	// division how classes
	std::vector<int> startVec;
	std::vector<int> endVec;
	int j = 0;
	for (int i = 0; i < veclen && j < veclen;)
	{
		start = i;
		startVec.push_back(start);
		for (j = i + 1; j < veclen; j++)
		{
			if (coreMemRatios[j] > divConst * coreMemRatios[start]) {
				end = j;
				endVec.push_back(end);
				classNumConst++;
				i = j;
				break;
			}
		}
		if (j == veclen)
		{
			end = j;
			endVec.push_back(end);
			classNumConst++;
			i = j;
		}
	}
	middleCoreVec.resize(classNumConst);
	ratioVec.resize(classNumConst);
	maxCoreVec.resize(classNumConst);
	maxMemVec.resize(classNumConst);

	for (int i = 0; i < classNumConst; i++)
	{
		start = startVec[i];
		end = endVec[i];
		if (start < 0)
			start = 0;
		if (end >= veclen)
			end = veclen;
		for (int j = start; j < end; j++)
			virtualListMap[helpVirtualList[ptr[j]].type].classFlag |= (1 << i);
		allCore = allMem = 0;
		maxCoreVec[i] = maxMemVec[i] = -1;

		helpCoreNumChange.resize(end - start);
		std::copy(helpCoreNum.begin() + start, helpCoreNum.begin() + end, helpCoreNumChange.begin());
		//======================middle==================
		int mid = (end - 1 + start) / 2 - start;
		int div = PartSort(helpCoreNumChange, 0, end - 1 - start);
		while (div != mid)
		{
			if (mid < div)   //左半区间找
				div = PartSort(helpCoreNumChange, 0, div - 1);
			else    //左半区间找
				div = PartSort(helpCoreNumChange, div + 1, end - 1 - start);
		}

		middleCoreVec[i] = helpCoreNumChange[mid];


		for (int j = start; j < end; j++)
		{
			allCore += helpVirtualList[ptr[j]].coreNum;
			allMem += helpVirtualList[ptr[j]].memNum;
			if (helpVirtualList[ptr[j]].duo)
			{
				if (maxCoreVec[i] < helpVirtualList[ptr[j]].coreNum)
					maxCoreVec[i] = helpVirtualList[ptr[j]].coreNum;
				if (maxMemVec[i] < helpVirtualList[ptr[j]].memNum)
					maxMemVec[i] = helpVirtualList[ptr[j]].memNum;
			}
			else
			{
				if (maxCoreVec[i] < helpVirtualList[ptr[j]].coreNum * 2)
					maxCoreVec[i] = helpVirtualList[ptr[j]].coreNum * 2;
				if (maxMemVec[i] < helpVirtualList[ptr[j]].memNum * 2)
					maxMemVec[i] = helpVirtualList[ptr[j]].memNum * 2;
			}
		}
		ratioVec[i] = double(allCore) / allMem;

	}
	free(ptr);
}

//============quest type===============
void QuestType::input()
{
	char c;
	std::string help;
	std::cin >> c >> help;                  //3.16
	help.pop_back();

	if (help == "add")
	{
		addOrDel = 1;
		std::cin >> type >> virtualId >> c;
		type.pop_back();
	}

	else if (help == "del")
	{
		addOrDel = 0;
		std::cin >> virtualId >> c;
	}
	else
		throw MyError("input error:not add or del", "QuestType::input");
}

//============quest list===============
void QuestList::set()
{
	std::cin >> T >> K;
	questVec.resize(T);
}
void QuestList::input()
{
	if (T == -1 || K == -1)
		throw ("set first", "QuestList::input");
	int r;
	std::cin >> r;
	questVec[nowDay].resize(r);
	for (int i = 0; i < r; i++)
		questVec[nowDay][i].input();
	nowDay++;
	if (nowDay > T)
		throw MyError("days out of range!", "QuestList::input");
}

void QuestList::input(const int& days)
{
	for (int i = 0; i < days; i++)
		input();
}

void QuestList::output(const int& day)
{
	for (const auto& e : questVec[day])
		if (e.addOrDel == 1) //if add, output
			if (e.inA == -1) //if duo
				std::cout << '(' << e.serverId << ')' << std::endl;
			else
				std::cout << '(' << e.serverId << ", " << (e.inA == 1 ? 'A' : 'B') << ')' << std::endl;
	fflush(stdout);
}


//====================nodeandheap.h=========================
class VirtualNode
{
public:

	bool duo;
	int coreNum, memNum;
	int id;

	int classFlag = -1;  //which class of server is.
	int classFlagAdd = -1;  //which class of server is.
	int migrationFlag = -1; //flag<days: can be migrated 

	int serverId;
	int inA; //inA:1 inB:0 duo:-1
	VirtualNode() {};
	VirtualNode(const int& mId, VirtualType virtualType);

	friend bool operator >= (const SingleServerNode& singleServerNode, const VirtualNode& virtualNode);
};

class VirtualHeap
{
private:
	std::map <int, VirtualNode> virtualHeap; //id and virtual node
public:
	const int size()const
	{
		return virtualHeap.size();
	};
	void add(const int& virtualId, const std::string& type)
	{
		virtualHeap[virtualId] = VirtualNode(virtualId, virtualList[type]);
	};
	void del(const int& virtualId) { virtualHeap.erase(virtualHeap.find(virtualId)); };
	VirtualNode& operator[] (const int& virtualId) { return virtualHeap[virtualId]; };
}virtualHeap;

struct RatioType
{
	int serverId;
	float ratio;
	RatioType(){serverId=-1;ratio=0;};
	RatioType(const ServerNode& serverNode);
	RatioType(const VirtualNode& virtualNode);
	friend bool operator< (const RatioType& a, const RatioType& b);
	friend bool operator== (const RatioType& a, const RatioType& b);
	friend bool operator> (const RatioType& a, const RatioType& b);
};

class SingleServerNode
{
private:
	std::deque<int> virtualIds; //only single virtual node
	int coreNum, memNum;
	int usedCoreNum, usedMemNum;
	friend class ServerNode;
public:
	SingleServerNode() {};
	SingleServerNode(const ServerType& serverType);
	const int popVirtual();
	void setId(const int& id);
	SingleServerNode& operator+= (const VirtualNode& virtualNode);
	SingleServerNode& operator-= (const VirtualNode& virtualNode);
	friend bool operator >= (const SingleServerNode& singleServerNode, const VirtualNode& virtualNode);
	friend bool remainCoreLess(const SingleServerNode& a, const SingleServerNode& b);
	friend bool remainMemLess(const SingleServerNode& a, const SingleServerNode& b);
	friend const SingleServerNode operator+ (const SingleServerNode& a, const SingleServerNode& b);

};

class ServerNode
{
private:
	std::deque<int> virtualIds; //only duo virtual node
	SingleServerNode a, b;
	int classFlag;  //which class of server is.
	std::vector<std::pair<int, int>> questReplyList; //day, index
public:
	int serverId = -1;
	std::string type;
	const int remainCore()const { return a.coreNum - a.usedCoreNum + b.coreNum - b.usedCoreNum; };
	const int remainMem()const { return a.memNum - a.usedMemNum + b.memNum - b.usedMemNum; };
	const int usedAdd()const { return a.usedCoreNum + b.usedCoreNum; };
	const int popVirtual();
	const int allCoreMem();
	const int usedCoreMem();
	const double getFullRatio();
	const bool empty();
	const float getRatio()const;
	ServerNode(const int& mServerId, const ServerType& serverType) :serverId(mServerId), a(serverType), b(serverType), classFlag(serverType.classFlag), type(serverType.type) { questReplyList.clear(); };
	void setId(const int& id);
	void setQuestReply(const int& day, const int& index, const int& inA);
	void setQuestReply(const int& day, const int& index);
	bool halfEmpty() { return (a.usedCoreNum + a.usedMemNum + b.usedCoreNum + b.usedMemNum) < emptyConst * 2 * (a.coreNum + a.memNum); };
	ServerNode& operator+= (VirtualNode& virtualNode);
	ServerNode& operator-= (const VirtualNode& virtualNode);
	friend bool operator >= (const ServerNode& ServerNode, const VirtualNode& virtualNode);
};

class ServerHeap
{
private:
	std::set<RatioType> ratioSet;
	std::set<RatioType> emptySet;
public:
	std::vector<ServerNode> serverVec;  //serverId:0 ~ size-1
	std::vector<ServerNode> helpServerVec; //serverId::size-1 ~ size+sizehelp-1
	const int findAndPop(VirtualNode& virtualNode, std::set<RatioType>& ratioSet);
	void setAndOutput(const int& day); //sort helpServerVec and output buying
	int add(VirtualNode& virtualId, const int& day); //return :index in helpServerVec
	void del(const int& virtualId);
	ServerNode& buy(const VirtualNode& virtualNode, const int& day);

	double getFullRatio();
	void migration(int& maxMigrationNum);
}serverHeap;

//============add and del===============
void add(QuestType& quest, const int& day, const int& questIndex)
{
	virtualHeap.add(quest.virtualId, quest.type); //virtual first
	auto& virtualNode = virtualHeap[quest.virtualId];
	int helpServerId = serverHeap.add(virtualNode, day);

	quest.serverId = virtualNode.serverId;
	quest.inA = virtualNode.inA;

	if (helpServerId >= 0)
		serverHeap.helpServerVec[helpServerId].setQuestReply(day, questIndex, virtualNode.inA);
}

void del(const QuestType& quest)
{
	serverHeap.del(quest.virtualId); //server first
	virtualHeap.del(quest.virtualId);
}

inline bool cmpHelpServer(const int& a, const int& b)
{
	return serverHeap.helpServerVec[a].type < serverHeap.helpServerVec[b].type;
}

//============server heap===============
bool cmpMig(const int& a, const int& b)
{
	return serverHeap.serverVec[a].getFullRatio() < serverHeap.serverVec[b].getFullRatio();
}
double ServerHeap::getFullRatio()
{
	int allCoreMem=0, usedCoreMem=0;
	for(auto &e:serverVec)
	{
		allCoreMem += e.allCoreMem();
		usedCoreMem += e.usedCoreMem();
	}
	return double(usedCoreMem)/allCoreMem;
}
void ServerHeap::migration(int& maxMigrationNum)
{
	if(!maxMigrationNum)
		return ;
	double fullRatio = getFullRatio();
	if(fullRatio>0.92)
		return ;
	int veclen = ratioSet.size();
	int selectServer = (1-fullRatio)*veclen*1.2+1;
	int *ptr = (int*) malloc(sizeof(int)*veclen);
	auto it = ratioSet.begin();
	for(int i=0; i<veclen; i++)
		ptr[i] = (it++)->serverId;
	nth_element(ptr, ptr+selectServer, ptr+veclen, cmpMig);
	sort(ptr, ptr+selectServer, cmpMig);
	
	int virtualId;
	int helpInA;
	for(int i=0; i<selectServer && maxMigrationNum ; i++)
	{
		if(serverVec[ptr[i]].empty())
		{
			ratioSet.erase(RatioType(serverVec[ptr[i]]));
			emptySet.insert(RatioType(serverVec[ptr[i]]));
			continue;
		}
		ServerNode& serverNode = serverVec[ptr[i]];
		ratioSet.erase(RatioType(serverNode));
		while((virtualId = serverNode.popVirtual()) != -1)
		{
			if(!maxMigrationNum)
				break;
			VirtualNode& virtualNode = virtualHeap[virtualId];
			helpInA = virtualNode.inA;
			if(this->add(virtualNode, -1)!=1e9) //add completed
			{
				//========
				virtualNode.inA ^= helpInA;
				helpInA ^= virtualNode.inA;
				virtualNode.inA ^= helpInA;
				//========
				serverNode -= virtualNode;
				//========
				virtualNode.inA ^= helpInA;
				helpInA ^= virtualNode.inA;
				virtualNode.inA ^= helpInA;
				//========
				migrationOutputVec.push_back(MigrationOutputType(virtualId, virtualNode.serverId, virtualNode.inA));
				maxMigrationNum--;
			}
			else
				break; //if not, put virtualId to the back of the queue in ServerNode
		}
		if(serverNode.empty())
			emptySet.insert(RatioType(serverNode));
		else
			ratioSet.insert(RatioType(serverNode));
	}

	free(ptr);
}

//void ServerHeap::migration(int& maxMigrationNum)
//{
//
//	int startIndex = getMigrationStart(maxMigrationNum);
//	//int startIndex = serverVec.size()-1;
//	int serverId, inA;
//	int nowVirtualId;
//	for (int i = startIndex; i >= 0 && maxMigrationNum > 0;)
//	{
//		nowVirtualId = serverVec[serverVecIdsAdd[i]].popVirtual();
//		if (nowVirtualId == -1)
//			i--;
//		else
//		{
//			delSort();
//			auto& nowVirtualNode = virtualHeap[nowVirtualId];
//			serverId = nowVirtualNode.serverId;
//			inA = nowVirtualNode.inA;
//			this->add(nowVirtualNode, 0);
//			if (serverId != nowVirtualNode.serverId || inA != nowVirtualNode.inA)
//			{
//				migrationOutputVec.push_back(MigrationOutputType(nowVirtualId, nowVirtualNode.serverId, nowVirtualNode.inA));
//				maxMigrationNum--;
//			}
//		}
//	}
//}

void ServerHeap::setAndOutput(const int& day)
{
	if (!helpServerVec.size())
	{
		std::cout << "(purchase, 0)" << std::endl;
		return;
	}
	//pop ratioSet
	for(auto &s: helpServerVec)
		ratioSet.erase(RatioType(s));

	//set virtual and quest
	int veclen = helpServerVec.size();
	int* helpServerVecIds = static_cast<int*>(malloc(sizeof(int) * veclen));
	for (int i = 0; i < veclen; i++)
		helpServerVecIds[i] = i;
	std::sort(helpServerVecIds, helpServerVecIds + veclen, cmpHelpServer);

	for (int i = 0; i < veclen; i++)
		helpServerVec[helpServerVecIds[i]].setId(i + serverVec.size());

	//set ratioSet
	for(auto &s: helpServerVec)
		ratioSet.insert(RatioType(s));

	//output
	string type = "";
	int typeNum = 0;
	for (int i = 0; i < veclen; i++)
		if (helpServerVec[helpServerVecIds[i]].type != type)
		{
			type = helpServerVec[helpServerVecIds[i]].type;
			typeNum++;
		}
	std::cout << "(purchase, " << typeNum << ')' << std::endl;

	type = helpServerVec[helpServerVecIds[0]].type;
	typeNum = 0;
	for (int i = 0; i < veclen; i++)
		if (helpServerVec[helpServerVecIds[i]].type != type)
		{
			std::cout << "(" << type << ", " << typeNum << ')' << std::endl;
			typeNum = 1;
			type = helpServerVec[helpServerVecIds[i]].type;
		}
		else
			typeNum++;
	std::cout << "(" << type << ", " << typeNum << ')' << std::endl;

	//move server node from helpServerVec to serverVec
	for (int i = 0; i < veclen; i++)
		serverVec.push_back(helpServerVec[helpServerVecIds[i]]);
	helpServerVec.clear();

	free(helpServerVecIds);
}

const int ServerHeap::findAndPop(VirtualNode& virtualNode, std::set<RatioType> &ratioSet)
{
	if(ratioSet.empty())
		return -1;
	auto itEnd = ratioSet.lower_bound(RatioType(virtualNode));
	auto itStart = itEnd;
	auto setStart = ratioSet.begin();
	auto setEnd = ratioSet.end();
	bool flagStart = (itStart != setStart); // itStart == setStart might be useful
	bool flagEnd = (itEnd != setEnd);

	while(flagStart||flagEnd)
	{
		if(flagEnd)
		{
			if(GETSERVER(itEnd->serverId)>=virtualNode)
			{
				ratioSet.erase(itEnd);
				return itEnd->serverId;
			}
			itEnd++;
			flagEnd = (itEnd != setEnd);
		}
		if(flagStart)
		{
			itStart--;
			if(GETSERVER(itStart->serverId)>=virtualNode)
			{
				ratioSet.erase(itStart);
				return itStart->serverId;
			}
			flagStart = (itStart != setStart);
		}
	}
	return -1;
}

int ServerHeap::add(VirtualNode& virtualNode, const int& day)
{
	int serverId = findAndPop(virtualNode, ratioSet);
	if(serverId != -1) //do not need to buy
	{
		ServerNode& serverNode = GETSERVER(serverId);
		serverNode += virtualNode;
		ratioSet.insert(RatioType(serverNode));
		return virtualNode.serverId - serverVec.size();
	}
	else if(day>=0)//need to buy
	{
		serverId = findAndPop(virtualNode, emptySet);
		if(serverId == -1)
		{
			auto &serverNode = buy(virtualNode, day);
			serverNode += virtualNode;
			ratioSet.insert(RatioType(serverNode));
			return virtualNode.serverId - serverVec.size();
		}
		else //used empty server
		{
			ServerNode& serverNode = GETSERVER(serverId);
			serverNode += virtualNode;
			ratioSet.insert(RatioType(serverNode));
			return virtualNode.serverId - serverVec.size();
		}

	}
	else //migration failed
		return 1e9;
}

void ServerHeap::del(const int& virtualId)
{
	const auto& virtualNode = virtualHeap[virtualId];

	ServerNode& serverNode = GETSERVER(virtualNode.serverId);
	ratioSet.erase(RatioType(serverNode));
	serverNode -= virtualNode;
	ratioSet.insert(RatioType(serverNode));
}

ServerNode& ServerHeap::buy(const VirtualNode& virtualNode, const int& day)
{
	double nearRatio = -1;
	int minClass = -1;
	double nowServerRatio;
	double nowVirtualRatio;
	for (int i = 0; i < classNumConst; i++)
	{
		if (virtualNode.classFlag & (1 << i))
		{
			auto& serverType = serverList.getServer(i, day);
			nowServerRatio = double(serverType.coreNum) / serverType.memNum;
			nowVirtualRatio = double(virtualNode.coreNum) / virtualNode.memNum;
			if (nearRatio == -1 || abs(log(nowServerRatio / nowVirtualRatio)) < abs(log(nearRatio / nowVirtualRatio)))
			{
				nearRatio = nowServerRatio;
				minClass = i;
			}
		}
	}
	if (minClass == -1)
		throw MyError("buy error", "ServerHeap::buy");
	int serverId = serverVec.size() + helpServerVec.size();
	const ServerType& serverType = serverList.getServer(minClass, day);
	helpServerVec.push_back(ServerNode(serverId, serverType));

	//*****Do no insert: insert in add()
	//ratioSet.insert(RatioType(helpServerVec.back()));
	return helpServerVec.back();
}

//============single server node===============
SingleServerNode::SingleServerNode(const ServerType& serverType) :coreNum(serverType.coreNum / 2), memNum(serverType.memNum / 2)
{
	virtualIds.clear();
	usedCoreNum = 0;
	usedMemNum = 0;
}

const int SingleServerNode::popVirtual()
{
	if(!virtualIds.empty())
	{
		VirtualNode& virtualNode = virtualHeap[virtualIds.front()];
		if(virtualNode.migrationFlag<static_days)
		{
			//virtualNode.migrationFlag = static_days;
			return virtualNode.id;
		}
	}
	return -1;
}

void SingleServerNode::setId(const int& id)
{
	for (auto& e : virtualIds)
		virtualHeap[e].serverId = id;
}

const SingleServerNode operator+ (const SingleServerNode& a, const SingleServerNode& b)
{
	SingleServerNode ans;
	ans.coreNum = a.coreNum + b.coreNum;
	ans.memNum = a.memNum + b.memNum;
	ans.usedCoreNum = a.usedCoreNum + b.usedCoreNum;
	ans.usedMemNum = a.usedMemNum + b.usedMemNum;
	return ans;
}

SingleServerNode& SingleServerNode::operator+= (const VirtualNode& virtualNode)
{
	if (virtualNode.duo)
	{
		usedCoreNum += virtualNode.coreNum / 2;
		usedMemNum += virtualNode.memNum / 2;
	}
	else
	{
		usedCoreNum += virtualNode.coreNum;
		usedMemNum += virtualNode.memNum;
		virtualIds.push_back(virtualNode.id);
	}
	if (virtualNode.duo)
		if (usedCoreNum > coreNum || usedMemNum > memNum)
			throw MyError("add error!", "SingleServerNode::operator+=");
	return *this;
}

SingleServerNode& SingleServerNode::operator-= (const VirtualNode& virtualNode)
{
	if (virtualNode.duo)
	{
		usedCoreNum -= virtualNode.coreNum / 2;
		usedMemNum -= virtualNode.memNum / 2;
	}
	else
	{
		usedCoreNum -= virtualNode.coreNum;
		usedMemNum -= virtualNode.memNum;
		if (virtualNode.id == virtualIds.front())
			virtualIds.pop_front();
		else
			virtualIds.erase(find(virtualIds.begin(), virtualIds.end(), virtualNode.id));
	}
	if (usedCoreNum < 0 || usedMemNum < 0)
		throw MyError("del error!", "SingleServerNode::operator-=");
	return *this;
}


bool operator >= (const SingleServerNode& singleServerNode, const VirtualNode& virtualNode)
{
	if (virtualNode.duo)
		return singleServerNode.coreNum - singleServerNode.usedCoreNum >= virtualNode.coreNum / 2 && singleServerNode.memNum - singleServerNode.usedMemNum >= virtualNode.memNum / 2;
	else
		return singleServerNode.coreNum - singleServerNode.usedCoreNum >= virtualNode.coreNum && singleServerNode.memNum - singleServerNode.usedMemNum >= virtualNode.memNum;
}

bool remainCoreLess(const SingleServerNode& a, const SingleServerNode& b)
{
	if (a.coreNum - a.usedCoreNum == b.coreNum - b.usedCoreNum)
		return a.memNum - a.usedMemNum < b.memNum - b.usedMemNum;
	return a.coreNum - a.usedCoreNum < b.coreNum - b.usedCoreNum;
}

bool remainMemLess(const SingleServerNode& a, const SingleServerNode& b)
{
	if (a.memNum - a.usedMemNum == b.memNum - b.usedMemNum)
		return a.memNum - a.usedMemNum < b.memNum - b.usedMemNum;
	return a.memNum - a.usedMemNum < b.memNum - b.usedMemNum;
}

//============ratio type===============
RatioType::RatioType(const ServerNode& serverNode){serverId=serverNode.serverId;ratio=serverNode.getRatio();}
RatioType::RatioType(const VirtualNode& virtualNode){serverId=0;ratio=float(virtualNode.coreNum)/virtualNode.memNum;}//memNum cannot be 0
bool operator< (const RatioType& a, const RatioType& b)
{
	return a.ratio < b.ratio || (a.ratio==b.ratio && a.serverId < b.serverId);
}
bool operator== (const RatioType& a, const RatioType& b)
{
	//a.serverId == b.serverId is very important
	return a.ratio == b.ratio && a.serverId == b.serverId; 
}
bool operator> (const RatioType& a, const RatioType& b)
{
	return a.ratio > b.ratio || (a.ratio==b.ratio && a.serverId > b.serverId);
}
//============server node===============


const int ServerNode::popVirtual()
{
	//migrationFlag;
	if(!virtualIds.empty())
	{
		VirtualNode& virtualNode = virtualHeap[virtualIds.front()];
		if(virtualNode.migrationFlag<static_days)
		{
			//virtualNode.migrationFlag = static_days;
			return virtualNode.id;
		}
	}
	int virtualId;
	if((virtualId = a.popVirtual())!=-1)
		return virtualId;
	else
		return b.popVirtual();
}

const int ServerNode::allCoreMem()
{
	return a.coreNum+a.memNum+b.coreNum+b.memNum;
}
const int ServerNode::usedCoreMem()
{
	return a.usedCoreNum+a.usedMemNum+b.usedCoreNum+b.usedMemNum;
}

const double ServerNode::getFullRatio()
{
	return (a.usedCoreNum+a.usedMemNum+b.usedCoreNum+b.usedMemNum)/double(a.coreNum+a.memNum+b.coreNum+b.memNum);
}

const bool ServerNode::empty()
{
	return a.usedCoreNum+a.usedMemNum+b.usedCoreNum+b.usedMemNum==0;
}

const float ServerNode::getRatio()const
{
	if(a.memNum+b.memNum-a.usedMemNum-b.usedMemNum == 0)
		return 1e10;
	return float(a.coreNum+b.coreNum-a.usedCoreNum-b.usedCoreNum)/(a.memNum+b.memNum-a.usedMemNum-b.usedMemNum);
}
void ServerNode::setId(const int& id)
{
	serverId = id;
	for (auto& e : virtualIds)
		virtualHeap[e].serverId = serverId;
	for (auto& e : questReplyList)
		questList[e.first][e.second].serverId = serverId;
	questReplyList.clear();
	a.setId(id);
	b.setId(id);
}

void ServerNode::setQuestReply(const int& day, const int& index, const int& inA)
{
	questReplyList.push_back(std::pair<int, int>(day, index));
}

ServerNode& ServerNode::operator+= (VirtualNode& virtualNode)
{
	virtualNode.serverId = serverId;
	if (virtualNode.duo)
	{
		a += virtualNode;
		b += virtualNode;
		virtualIds.push_back(virtualNode.id);
		virtualNode.inA = -1;
	}
	else
	{
		if (virtualNode.coreNum <= virtualNode.memNum)
			if (a >= virtualNode && (remainCoreLess(a, b) || !(b >= virtualNode)))
			{
				a += virtualNode;
				virtualNode.inA = 1;
			}
			else
			{
				b += virtualNode;
				virtualNode.inA = 0;
			}
		else
			if (a >= virtualNode && (remainMemLess(a, b) || !(b >= virtualNode)))
			{
				a += virtualNode;
				virtualNode.inA = 1;
			}
			else
			{
				b += virtualNode;
				virtualNode.inA = 0;
			}

	}
	return *this;
}

bool operator >= (const ServerNode& serverNode, const VirtualNode& virtualNode)
{
	if (virtualNode.duo)
		return serverNode.a >= virtualNode && serverNode.b >= virtualNode;
	else
		return serverNode.a >= virtualNode || serverNode.b >= virtualNode;
}

ServerNode& ServerNode::operator-= (const VirtualNode& virtualNode)
{
	if (virtualNode.duo)
	{
		a -= virtualNode;
		b -= virtualNode;
		if (virtualNode.id == virtualIds.front())
			virtualIds.pop_front();
		else
			virtualIds.erase(find(virtualIds.begin(), virtualIds.end(), virtualNode.id));
	}
	else
		if (virtualNode.inA) //inA should not be -1
			a -= virtualNode;
		else
			b -= virtualNode;
	return *this;
}

//============virtual node===============
VirtualNode::VirtualNode(const int& mId, VirtualType virtualType) :id(mId)
{
	duo = virtualType.duo;
	coreNum = virtualType.coreNum;
	memNum = virtualType.memNum;
	serverId = inA = -2;
	classFlag = virtualType.classFlag;
};

//====================io.h=========================
void input()
{
	serverList.input();
	virtualList.input();
	questList.set();
	questList.input(questList.K);
	serverList.setDays(questList.T); //alldays
}

void migrationOutput()
{
	std::cout << "(migration," << migrationOutputVec.size() << ')' << std::endl;
	for (const auto& mig : migrationOutputVec)
		if (mig.inA == -1)
			std::cout << '(' << mig.virtualId << ',' << mig.serverId << ')' << std::endl;
		else
			std::cout << '(' << mig.virtualId << ',' << mig.serverId << ',' << (mig.inA ? 'A' : 'B') << ')' << std::endl;
	migrationOutputVec.clear();
}

//====================main.cpp=========================
int main()
{
	//freopen("training-2.txt", "r", stdin);
	//freopen("out2.txt", "w", stdout);
	input();
	int maxMigrationNum;
	const int allDays = questList.T;
	for (int i = 0; i < allDays; i++)
	{



		static_days = i;
		int addnum = 0, delnum = 0;
		//cerr << "day" << i << endl;
		maxMigrationNum = virtualHeap.size() * 3 / 100;
		serverHeap.migration(maxMigrationNum);



		vector<QuestType>& questThisDay = questList[i];
		const int veclen = questThisDay.size();
		for (int j = 0; j < veclen; j++)
			if (questThisDay[j].addOrDel)
				add(questThisDay[j], i, j);
			else
				del(questThisDay[j]);


		serverHeap.setAndOutput(i);
		migrationOutput();
		questList.output(i);

		if (questList.T - questList.K - i > 0)
			questList.input();

	}


	return 0;
}
