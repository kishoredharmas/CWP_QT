# 🚦 GitHub Copilot Instructions — CWP Qt Modernization Project

## 📌 Project Overview

This project modernizes a legacy Controller Working Position (CWP) system by:

- Replacing the legacy **ODS Toolbox UI** with **Qt (C++)**
- Replacing the **Recording & Playback System (RPS)** with a custom in-house solution
- Following **Clean Architecture**, **SOLID**, **MISRA-inspired guidelines**, and **Clean Code principles**

---

## 🧱 Core Architecture

```
+-------------------------------+
| Presentation (Qt UI)          |
+-------------------------------+
| Application (Use Cases)       |
+-------------------------------+
| Domain (Entities / Models)    |
+-------------------------------+
| Infrastructure (IO, Qt, File) |
+-------------------------------+
```

---

## 🧩 Coding Language

- **C++17 or newer**
- Qt framework (Qt6 preferred)

---

## 📐 Design Principles

### ✅ SOLID Principles

- Single Responsibility Principle
- Open/Closed Principle
- Liskov Substitution Principle
- Interface Segregation Principle
- Dependency Inversion Principle

---

## 🧹 Clean Code Guidelines

- Use meaningful names
- Keep functions small
- Avoid redundant comments

---

## ⚖️ MISRA-Inspired Rules

- Avoid raw pointers
- Always initialize variables
- Avoid magic numbers
- Use const correctness

---

## 📁 Project Structure

```
/domain
/application
/infrastructure
/ui
/tests
```

---

## 📚 Documentation Rules

- Every class and function must have Doxygen-style comments

---

## ✅ Copilot Guidance

- Follow SOLID, Clean Architecture, and modern C++ best practices
- Generate readable, testable, and maintainable code
