//
// Created by ashish on 4/4/17.
//

#include "../BasicScene.hpp"
#include <iostream>
int main(){



    std::ios_base::sync_with_stdio(false);
//    //TODO load scene here and send it to app via r value reference
    BasicScene s(1280,720,"Gpu Path Tracer");
    s.run();





    return 0;
}
