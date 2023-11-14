#include <iostream>
#include <string>
#include <bitset>
#include <sstream>

std::string binaryToHex(const std::string& binaryString) {
    std::bitset<64> bits(binaryString);
    std::stringstream ss;
    ss << std::hex << bits.to_ullong();
    return ss.str();
}

int printCANMessage(const std::string& hexValue){
    std::string binaryValue;

    for (char hexChar : hexValue) {
        switch (hexChar) {
            case '0': binaryValue += "0000"; break;
            case '1': binaryValue += "0001"; break;
            case '2': binaryValue += "0010"; break;
            case '3': binaryValue += "0011"; break;
            case '4': binaryValue += "0100"; break;
            case '5': binaryValue += "0101"; break;
            case '6': binaryValue += "0110"; break;
            case '7': binaryValue += "0111"; break;
            case '8': binaryValue += "1000"; break;
            case '9': binaryValue += "1001"; break;
            case 'A': case 'a': binaryValue += "1010"; break;
            case 'B': case 'b': binaryValue += "1011"; break;
            case 'C': case 'c': binaryValue += "1100"; break;
            case 'D': case 'd': binaryValue += "1101"; break;
            case 'E': case 'e': binaryValue += "1110"; break;
            case 'F': case 'f': binaryValue += "1111"; break;
            default:
                std::cout << "Invalid hexadecimal value." << std::endl;
                return 0;
        }
    }
    //Split CAN message into parts
    std::string functionCode = binaryValue.substr(0,4);
    std::string nodeID = binaryValue.substr(4, 7);
    std::string RTR = binaryValue.substr(11,1);
    std::string DataLength = binaryValue.substr(12,4);
    std::string DataPortion = binaryValue.substr(16,64);

    std::string DataPortion1 = binaryValue.substr(16,8);
    std::string DataPortion2 = binaryValue.substr(24,8);
    std::string DataPortion3 = binaryValue.substr(32,8);
    std::string DataPortion4 = binaryValue.substr(40,8);
    std::string DataPortion5 = binaryValue.substr(48,8);
    std::string DataPortion6 = binaryValue.substr(56,8);
    std::string DataPortion7 = binaryValue.substr(64,8);
    std::string DataPortion8 = binaryValue.substr(72,8);
    
    std::cout << std::endl;
    std::cout << "function  NodeID        RTR     DataLength      Data1           Data2           Data3           Data4           Data5           Data6           Data7           Data8" << std::endl;
    
    // Binary Output
    std::cout << functionCode << "\t  " << nodeID << "\t" << RTR << "\t" << DataLength << "\t\t" << DataPortion1 << "\t" << (DataPortion2)
    << "\t" << (DataPortion3) << "\t" << (DataPortion4) << "\t" << (DataPortion5)
    << "\t" << (DataPortion6) << "\t" << (DataPortion7) << "\t" << (DataPortion8) << std::endl;
    
    // Hex Output
    std::cout << binaryToHex(functionCode) << "\t  " << binaryToHex(nodeID) << "\t\t" << 
    binaryToHex(RTR) << "\t" << binaryToHex(DataLength) << "\t\t" << binaryToHex(DataPortion1) << "\t\t" << binaryToHex(DataPortion2)
    << "\t\t" << binaryToHex(DataPortion3) << "\t\t" << binaryToHex(DataPortion4) << "\t\t" << binaryToHex(DataPortion5)
    << "\t\t" << binaryToHex(DataPortion6) << "\t\t" << binaryToHex(DataPortion7) << "\t\t" << binaryToHex(DataPortion8) << std::endl;
}


int main() {
    // std::cout << "Enter a hexadecimal value: ";
    std::string hexValue = "01234567890123456789";
    //std::cin >> hexValue;
    printCANMessage(hexValue);
    return 0;
}