#pragma once
#include "child.h"
#include <iostream>
#include <string>

class Parent {
  public:
    Parent();

    void test();

    void log(const std::string &logText);

  private:
    Child child;
};