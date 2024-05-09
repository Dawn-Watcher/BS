#include "ns3/node.h"
#include "ns3/log.h"
#include <iostream>
#include <cstdlib>  // For system()
#include <iostream>
namespace ns3 {



class MyCustomNode : public Node {
public:
    MyCustomNode() : m_id(0), m_name("Default") {}
    void SetId(uint32_t id) { m_id = id; }
    void setModel(std::string model){model_address=model;}
    void SetName(std::string name) { m_name = name; }
    uint32_t GetId() const { return m_id; }
    std::string GetName() const { return m_name; }

    friend std::ostream& operator<<(std::ostream& os, const MyCustomNode& node);
    void run_model(uint32_t rank=0){
        const char* pathToScript = "/home/ysq/workspace/ns-3-allinone/ns-3.38/model/a.py";
        std::string command = "/home/ysq/anaconda3/envs/pytorch/bin/python "; // 或者 python3 ", 根据你的系统配置
        command += pathToScript;

        // 调用系统命令
        std::cout << "Executing Python script..." << std::endl;
        int result = system(command.c_str());

        if (result != 0) {
            std::cerr << "Error during the execution of the Python script" << std::endl;
        }

        
    }
private:
    uint32_t m_id;
    std::string m_name;
    std::string model_address;
    
};

// 实现输出操作符重载
std::ostream& operator<<(std::ostream& os, const MyCustomNode& node) {
    os << "Node(ID=" << node.GetId() << ", Name=" << node.GetName() << ")";
    return os;
}

} // namespace ns3
