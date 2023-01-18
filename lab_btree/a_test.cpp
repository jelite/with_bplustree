#include <vector>
#include <iostream>


using namespace std;

int main(){
    vector<int> a = {{1},{2},{3}};
    vector<int> b = {{11}, {22}, {33}, {44}, {55}};

    a.erase(a.begin());
    a.erase(a.begin());
    a.erase(a.begin());

    std::cout << a.size() << std::endl; 
    // for(auto it : a)
    // {
    //     std::cout << it << std::endl; 
    // }
}