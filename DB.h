#pragma once
#include <SQLiteCpp.h>
#include "Models.h"

std::string getDBPath();
void initializeDB();
std::shared_ptr<Runnable> SaveRunnable(Runnable runnable);
std::shared_ptr<Runnable> findRunnable(int64_t id);
std::shared_ptr<Runnable> UpdateRunnable(Runnable runnable);
bool deleteRunnable(int64_t id);