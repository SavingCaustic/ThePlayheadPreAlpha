#pragma once

// Forward declaration of Parent class to avoid circular dependency
class Parent;

class Child {
  public:
    // explicit Child(Parent *parent);
    explicit Child(Parent &parent);

    float myComplicatedFormula();

  private:
    // Parent *parent; // Pointer to Parent
    Parent &parent; // Pointer to Parent
};