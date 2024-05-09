#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

// 自定义应用，用于存储和处理数据
class MyDataApp : public Application {
public:
    MyDataApp() {}
    virtual ~MyDataApp() {}

    void SetData(std::string data) {
        m_data = data;
    }

    std::string GetData() const {
        return m_data;
    }

private:
    std::string m_data;
};

void getdata(Ptr<MyDataApp> app){
    std::cout << "Stored data: " << app->GetData() << std::endl;
};


int main() {
    Ptr<Node> node = CreateObject<Node>();
    Ptr<MyDataApp> app = CreateObject<MyDataApp>();
    app->SetData("Hello, ns-3!");

    node->AddApplication(app);

    Simulator::Schedule(Seconds(1),&getdata,app);
    Simulator::Run();
    Simulator::Destroy();

    std::cout << "Stored data: " << app->GetData() << std::endl;
    return 0;
}
