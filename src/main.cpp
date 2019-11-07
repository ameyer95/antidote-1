#include "ExperimentFrontend.h"
using namespace std;

int main(int argc, char **argv) {
    ExperimentFrontend e;
    if(e.processCommandLineArguments(argc, argv)) {
        e.performExperiments();
    }
    return 0;
}

