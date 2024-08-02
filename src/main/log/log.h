#ifndef HPROF_ANALYZER_SRC_MAIN_LOG_LOG_H_
#define HPROF_ANALYZER_SRC_MAIN_LOG_LOG_H_

#ifdef DEBUG_LOGS
#define log(str) std::cout << str << std::endl;
#endif

#ifndef DEBUG_LOGS
#define log(str)
#endif
#endif //HPROF_ANALYZER_SRC_MAIN_LOG_LOG_H_
