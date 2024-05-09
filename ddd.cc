#include <cstdlib>  // For system()
#include <iostream>
using namespace std;
int main() {
    const char* pathToScript = "/home/ysq/workspace/ns-3-allinone/ns-3.38/model/a.py";
    std::string command = "/home/ysq/anaconda3/envs/pytorch/bin/python "; // 或者 python3 ", 根据你的系统配置
    command += pathToScript;

    // 调用系统命令
    std::cout << "Executing Python script..." << std::endl;
    int result = system(command.c_str());

    if (result != 0) {
        std::cerr << "Error during the execution of the Python script" << std::endl;
    }

    return result;
}
