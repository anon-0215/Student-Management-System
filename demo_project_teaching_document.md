# 校园学生食堂信息管理系统 Demo 教学文档

> 这个文档不是提交作业用的报告，而是专门给你读代码、补 C++ 语法、理解项目结构用的教学文档。它配合 `project_bugfix_annotated.cpp` 一起看：代码文件负责逐行解释，本文档负责讲注释里放不下的背景知识、语法知识和函数块之间的关系。

---

## 1. 本次我实际改了哪里

原 demo 代码中，`dishManageMenu(vector<Dish>& dishes)` 的菜单显示了 4 个选项：

```cpp
cout << "1. 查看全部菜品\n";
cout << "2. 增加菜品\n";
cout << "3. 删除菜品\n";
cout << "4. 修改菜品\n";
cout << "0. 返回上一级\n";
```

但是读取菜单选择时原来写成：

```cpp
int choice = readInt("请选择：", 0, 3);
```

这表示用户只能输入 0、1、2、3，输入 4 会被判定为无效，所以“修改菜品”这个功能虽然写了 `case 4`，但是用户永远进不去。

因此唯一功能性修复是：

```cpp
int choice = readInt("请选择：", 0, 4);
```

也就是说：**菜单有 4 项，就允许输入到 4。** 除此之外，没有新增功能，没有改变数据结构，没有改变菜单逻辑。逐行注释版中新增的大量文字都是注释，不影响程序运行。

---

## 2. 你应该按什么顺序读这个项目

不要从 `main()` 一路硬读到上面，也不要从第一行开始强行背。建议按下面顺序读：

1. 先看“数据长什么样”：`Dish`、`Order`、`Student`。
2. 再看“学生链表怎么管理”：`StudentList`。
3. 再看“工具函数”：`readInt`、`readDouble`、`readText`、`showAllDishes`。
4. 再看“学生能做什么”：`Student::viewDishes`、`Student::placeOrder`、`Student::pickupOrder`、`Student::showOrders`。
5. 再看“管理员能做什么”：`adminAddStudent`、`adminDeleteStudent`、`adminModifyDish`、`adminShowAllOrders` 等。
6. 最后看菜单怎么串起来：`mainMenu` → `adminMenu/studentMenu` → 各级子菜单。
7. 最后才看 `main()`：它只负责创建数据、初始化数据、进入主菜单。

一句话：**先看数据，再看操作，再看菜单，最后看入口。**

---

## 3. 这个项目的核心思想

这个 demo 不是为了炫技，而是为了满足题目 1 的重点：

- 学生信息必须用“学生类链表”管理。
- 管理员是超级用户，主要通过全局函数管理系统整体数据。
- 学生登录后，主要通过学生对象自己的成员函数完成自己的功能。
- 代码要体现“成员函数”和“全局函数”的区别。

### 3.1 成员函数和全局函数的区别

成员函数长这样：

```cpp
void Student::placeOrder(vector<Dish>& dishes, int& nextOrderId)
```

它属于某一个学生对象。比如：

```cpp
student->placeOrder(dishes, nextOrderId);
```

这里 `student` 指向的具体学生就是“调用者对象”。这个函数内部可以直接访问该学生的 `studentId`、`name`、`orders` 等私有数据。虽然代码里没写 `this->orders`，但本质上就是在操作 `this` 指向的那个学生。

全局函数长这样：

```cpp
void adminAddStudent(StudentList& students)
```

它不属于某一个学生对象，也没有 `this` 指针。它接收整个学生链表 `students`，站在系统管理员角度做全局管理。

可以这样理解：

| 类型 | 谁调用 | 操作范围 | 典型例子 |
|---|---|---|---|
| 学生成员函数 | 某个学生对象 | 主要管理这个学生自己的信息和订单 | `student->placeOrder()` |
| 管理员全局函数 | 菜单函数直接调用 | 管理整个系统的数据 | `adminAddStudent(students)` |

---

## 4. 程序运行流程图式理解

程序大致运行过程如下：

```text
main()
  │
  ├─ 创建 StudentList students
  ├─ 创建 vector<Dish> dishes
  ├─ 设置 nextOrderId = 1
  ├─ initStudents(students)
  ├─ initDishes(dishes)
  └─ mainMenu(students, dishes, nextOrderId)
          │
          ├─ 1 管理员登录
          │      └─ adminMenu()
          │             ├─ 学生数据管理
          │             ├─ 菜品管理
          │             └─ 订单查询
          │
          ├─ 2 学生登录
          │      └─ studentMenu()
          │             ├─ 查看菜品
          │             ├─ 在线订餐
          │             ├─ 取餐验证
          │             ├─ 查询我的订单
          │             └─ 查看个人信息
          │
          ├─ 3 学生注册
          │      └─ registerStudent()
          │
          └─ 0 退出系统
```

这个结构体现了“多级菜单”：主菜单下面有管理员菜单和学生菜单，管理员菜单下面又有学生管理、菜品管理、订单查询等子菜单。

---

## 5. 数据结构详解

### 5.1 `Dish`：菜品结构体

`Dish` 表示一道菜。它用 `struct` 定义，因为它主要是“数据包”，没有复杂行为。

关键字段：

| 字段 | 含义 |
|---|---|
| `id` | 菜品编号，点餐时靠它选择菜品 |
| `name` | 菜品名称 |
| `category` | 分类，如早餐、午餐、晚餐、特色菜 |
| `price` | 单价 |
| `stock` | 库存数量 |
| `flavor` | 口味说明 |
| `nutrition` | 营养成分 |
| `allergen` | 过敏源 |
| `rating` | 评分 |

本 demo 用 `vector<Dish>` 保存所有菜品。`vector` 可以理解为“会自动变长的数组”。

### 5.2 `Order`：订单结构体

`Order` 表示一个订单。每个学生对象内部有一个 `vector<Order> orders;`，所以订单是跟着学生保存的。

关键字段：

| 字段 | 含义 |
|---|---|
| `orderId` | 订单编号 |
| `dishId` | 对应菜品编号 |
| `dishName` | 下单时的菜品名 |
| `quantity` | 数量 |
| `amount` | 总金额 |
| `mealTime` | 用餐时段 |
| `mode` | 堂食、自提、外卖 |
| `status` | 已预订或已取餐 |
| `pickupCode` | 取餐码 |

这里有一个设计细节：订单里保存了 `dishName`，而不是只保存 `dishId`。这样即使管理员后来修改了菜品名字，历史订单仍然能显示当时下单的菜名。

### 5.3 `OrderStatus`：订单状态枚举

代码中写：

```cpp
enum class OrderStatus
{
    Reserved,
    PickedUp
};
```

`enum class` 是“强类型枚举”。你可以把它理解成：订单状态只能从几个固定选项里选。比直接用 0、1 更清楚。

- `OrderStatus::Reserved`：已预订。
- `OrderStatus::PickedUp`：已取餐。

然后通过：

```cpp
orderStatusToString(order.status)
```

把枚举状态转换成中文显示。

### 5.4 `Student`：学生类

`Student` 是这个项目最重要的类。它同时承担两件事：

1. 保存学生基本信息。
2. 保存这个学生自己的订单。

它的私有成员包括：

```cpp
string studentId;
string password;
string name;
string gender;
string birthDate;
string grade;
string major;
string phone;
vector<Order> orders;
```

这些字段被放在 `private` 里，表示外部不能直接改。外部如果想看学号，就调用：

```cpp
student->getId();
```

如果想让学生订餐，就调用：

```cpp
student->placeOrder(dishes, nextOrderId);
```

这就是面向对象中的“把数据和操作数据的函数放在一起”。

### 5.5 `Student* next`：为什么学生类里有指针

`Student` 类里有：

```cpp
Student* next;
```

这是链表的关键。一个学生对象不仅保存自己的信息，还保存“下一个学生在哪里”。

链表可以画成这样：

```text
head
 │
 ▼
[学生A | next] ──▶ [学生B | next] ──▶ [学生C | next] ──▶ nullptr
```

`nullptr` 表示后面没有学生了。

---

## 6. 学生链表 `StudentList` 详解

`StudentList` 负责管理整条学生链表。它内部只有一个核心数据：

```cpp
Student* head;
```

`head` 是头指针。链表所有操作基本都从 `head` 开始。

### 6.1 添加学生：头插法

添加学生时，代码逻辑是：

```cpp
newStudent->next = head;
head = newStudent;
```

如果原来链表是：

```text
head ──▶ A ──▶ B ──▶ C ──▶ nullptr
```

现在要插入新学生 X：

第一步：`newStudent->next = head;`

```text
X ──▶ A ──▶ B ──▶ C ──▶ nullptr
head ──▶ A
```

第二步：`head = newStudent;`

```text
head ──▶ X ──▶ A ──▶ B ──▶ C ──▶ nullptr
```

这叫“头插法”。优点是代码短、速度快，不需要遍历到链表尾部。

### 6.2 查找学生

查找学生靠循环：

```cpp
Student* current = head;
while (current != nullptr)
{
    if (current->getId() == studentId)
    {
        return current;
    }
    current = current->next;
}
return nullptr;
```

意思是：从头结点开始，一个一个往后看。如果学号相同，就返回这个学生对象的指针。找完整条链表还没找到，就返回空指针 `nullptr`。

### 6.3 删除学生

删除学生比添加和查找更难，因为要把前一个结点接到后一个结点上。

代码用两个指针：

- `current`：当前正在检查的学生。
- `previous`：current 前面的那个学生。

删除时分两种情况：

1. 删除的是头结点：直接让 `head = current->next`。
2. 删除的不是头结点：让 `previous->next = current->next`。

最后必须：

```cpp
delete current;
```

因为学生对象是用 `new Student(...)` 创建的，不 delete 会造成内存泄漏。

---

## 7. 输入函数为什么要自己封装

代码没有到处直接写 `cin >> choice;`，而是封装了：

- `readInt`
- `readDouble`
- `readText`

这样做的好处是：所有输入错误处理都集中在一起。

### 7.1 `readInt`

作用：读取一个整数，并要求它在指定范围内。

例如：

```cpp
int choice = readInt("请选择：", 0, 4);
```

意思是：用户必须输入 0 到 4 之间的整数。输入字母、负数、超过范围的数，都会提示重新输入。

关键语法：

```cpp
if (cin >> value && value >= minValue && value <= maxValue)
```

这行同时检查三件事：

1. `cin >> value` 是否成功读取整数。
2. `value >= minValue` 是否不小于最小值。
3. `value <= maxValue` 是否不大于最大值。

三个条件都成立，才返回输入值。

### 7.2 为什么需要 `clearInput()`

当用户本来应该输入整数，却输入了 `abc`，`cin` 会进入错误状态。此时如果不清理，后面的输入也可能继续失败。

所以代码中用了：

```cpp
cin.clear();
cin.ignore(numeric_limits<streamsize>::max(), '\n');
```

可以理解为两步：

1. `cin.clear()`：解除错误状态。
2. `cin.ignore(...)`：丢掉这一行剩下的垃圾输入。

### 7.3 `readText`

`readText` 用 `getline(cin, text)` 读取整行文本。它适合读取姓名、专业、菜名这种可能包含中文和空格的内容。

`allowEmpty` 表示是否允许空字符串。管理员修改学生信息时，允许直接回车保留原内容，所以会传 `true`。

---

## 8. 菜品相关函数块

### 8.1 `findDishIndexById`

这个函数遍历 `vector<Dish>`，找到指定菜品编号对应的下标。

为什么返回下标，而不是直接返回菜品？因为订餐时需要修改库存：

```cpp
Dish& selectedDish = dishes[index];
selectedDish.stock -= quantity;
```

如果只返回一个复制出来的菜品，修改它不会影响原数组。返回下标后，再用引用 `Dish&` 操作原来的菜品，就能真正扣库存。

### 8.2 `getNextDishId`

这个函数找出当前最大菜品编号，然后加 1，作为新菜品编号。

例如已有 1001、1002、1006，那么新编号就是 1007。

### 8.3 `showDishBrief` 和 `showDishDetail`

二者区别：

| 函数 | 用途 |
|---|---|
| `showDishBrief` | 用于表格列表，显示菜品核心信息 |
| `showDishDetail` | 用于查看单个菜品详情，显示营养、过敏源等 |

`showDishBrief` 里用了：

```cpp
if (dish.stock <= LOW_STOCK_LINE)
{
    cout << "  库存偏低";
}
```

这就是一个简单的低库存预警。

---

## 9. 学生功能函数块

### 9.1 `Student::viewDishes`

这个函数让当前学生查看菜单。虽然菜品列表是全局数据，但是“查看”这个动作由当前学生对象发起，所以设计成学生成员函数。

流程：

1. 输出“某某正在查看今日菜单”。
2. 调用 `showAllDishes(dishes)` 显示所有菜品。
3. 询问是否查看某个菜品详情。
4. 如果输入菜品编号，就调用 `findDishIndexById` 查找。
5. 找到则显示详情，找不到则提示。

### 9.2 `Student::placeOrder`

这是订餐核心函数。

流程：

```text
检查有没有菜品
  ↓
显示菜品列表
  ↓
输入菜品编号
  ↓
检查菜品是否存在
  ↓
检查库存是否足够
  ↓
输入订购数量
  ↓
选择用餐时段
  ↓
选择用餐方式
  ↓
生成 Order 对象
  ↓
orders.push_back(newOrder)
  ↓
selectedDish.stock -= quantity
  ↓
输出订单信息和取餐码
```

最关键的两行：

```cpp
orders.push_back(newOrder);
selectedDish.stock -= quantity;
```

第一行表示“把订单加入当前学生自己的订单列表”。第二行表示“从菜品库存中扣掉对应数量”。

### 9.3 `Student::pickupOrder`

取餐验证的核心是防止重复领取。

判断顺序：

1. 如果当前学生没有订单，直接返回。
2. 让学生输入订单号和取餐码。
3. 在自己的订单里查找订单号。
4. 如果订单已经是 `PickedUp`，提示不能重复领取。
5. 如果取餐码不对，提示验证失败。
6. 如果都正确，把状态改成 `PickedUp`。

注意：它只查当前学生自己的 `orders`，所以学生不能取别人的订单。

### 9.4 `Student::showOrders`

这个函数只显示当前学生自己的订单。管理员如果要查全部订单，会遍历学生链表，然后调用或读取每个学生的订单。

---

## 10. 管理员功能函数块

管理员函数大多是全局函数，因为管理员不是某一个学生对象。

### 10.1 学生管理

| 函数 | 作用 |
|---|---|
| `createStudentFromInput` | 从键盘读取学生信息，并 new 一个学生对象 |
| `adminAddStudent` | 添加学生，检查学号是否重复 |
| `adminDeleteStudent` | 删除学生，删除前二次确认 |
| `adminSearchStudent` | 查询学生个人资料和订单 |
| `adminModifyStudent` | 找到学生后调用该学生的 `modifyByAdmin` |
| `studentManageMenu` | 学生管理子菜单 |

这里有一个很好的面向对象设计点：管理员菜单是全局函数，但真正修改学生对象内部数据时，调用了：

```cpp
student->modifyByAdmin();
```

也就是说，管理员负责“找到这个学生”，而学生对象自己的成员函数负责“修改自己内部的数据”。

### 10.2 菜品管理

| 函数 | 作用 |
|---|---|
| `adminAddDish` | 自动生成编号，录入新菜品 |
| `adminDeleteDish` | 按编号删除菜品 |
| `adminModifyDish` | 按编号修改菜品信息 |
| `dishManageMenu` | 菜品管理子菜单 |

本次 bug 就在 `dishManageMenu`：菜单有第 4 项，但输入范围原来只允许到 3。

### 10.3 订单查询

| 函数 | 作用 |
|---|---|
| `adminShowAllOrders` | 遍历所有学生，再遍历每个学生的订单 |
| `adminSearchOrdersByStudent` | 按学号查某个学生的订单 |
| `adminSearchOrdersByDish` | 按菜品名关键字查有哪些学生订购 |
| `orderQueryMenu` | 订单查询子菜单 |

`adminShowAllOrders` 的遍历逻辑是两层：

```text
学生链表：李明 → 王雨 → 张辰
  每个学生内部：订单1、订单2、订单3...
```

因此它先遍历学生，再遍历当前学生的订单。

---

## 11. 菜单函数块

菜单函数的结构都差不多：

```cpp
while (true)
{
    显示菜单;
    int choice = readInt(...);
    if (choice == 0) return;

    switch (choice)
    {
        case 1: 调用功能1; break;
        case 2: 调用功能2; break;
        ...
    }
    pauseScreen();
}
```

这里 `while (true)` 表示这个菜单会一直显示，直到用户输入 0。输入 0 后执行 `return`，从当前菜单函数返回上一级菜单。

### 11.1 为什么用 `switch`

因为菜单选项是固定整数，例如 1、2、3、4。用 `switch` 比很多个 `if-else` 更清楚。

### 11.2 `pauseScreen()` 的意义

如果不暂停，执行完功能后马上刷新菜单，用户可能还没看清结果。`pauseScreen()` 就是让用户按回车后再继续。

---

## 12. 你可能不熟的新语法解释

### 12.1 `vector<T>`

`vector<Dish>` 可以理解成“装 Dish 的动态数组”。

常用操作：

| 写法 | 意思 |
|---|---|
| `dishes.push_back(dish)` | 在末尾加入一个菜品 |
| `dishes.size()` | 获取菜品数量 |
| `dishes.empty()` | 判断是否为空 |
| `dishes[i]` | 访问第 i 个元素 |
| `dishes.erase(dishes.begin() + index)` | 删除某个下标的元素 |

### 12.2 引用 `&`

代码里经常出现：

```cpp
void adminAddDish(vector<Dish>& dishes)
```

`&` 表示引用。这里的意思是函数操作的是原来的 `dishes`，不是复制品。这样函数内部添加菜品后，外面的菜品列表也会变。

如果没有 `&`，函数拿到的是副本，改了也不会影响外面。

### 12.3 `const` 引用

例如：

```cpp
void showAllDishes(const vector<Dish>& dishes)
```

它有两个意思：

1. `&`：不复制整个菜品数组，提高效率。
2. `const`：函数只读，不允许修改菜品数组。

所以它适合“显示数据”这种只读操作。

### 12.4 指针 `*` 和箭头 `->`

```cpp
Student* student = studentLogin(students);
student->showOrders();
```

`Student*` 表示 student 是一个指针，它保存的是学生对象的地址。

通过指针调用成员函数时，用 `->`。

如果是普通对象，用点号：

```cpp
StudentList students;
students.showAllStudents();
```

如果是对象指针，用箭头：

```cpp
Student* student;
student->showProfile();
```

### 12.5 `new` 和 `delete`

```cpp
Student* newStudent = new Student(...);
```

`new` 会在堆区创建对象，并返回指针。因为链表结点需要长期存在，不能在函数结束后自动消失，所以使用 `new`。

对应地，删除学生或清空链表时要：

```cpp
delete current;
```

否则内存不会释放。

### 12.6 构造函数初始化列表

代码中：

```cpp
Student(...) 
    : studentId(id),
      password(pwd),
      name(stuName),
      ...
      next(nullptr) {}
```

冒号后面这部分叫初始化列表。它的作用是在对象创建时直接初始化成员变量。

你可以先把它理解成更高效、更规范的赋值方式。它等价于“创建学生时把传进来的 id、pwd、name 等保存到对象内部”。

### 12.7 范围 for 循环

```cpp
for (const Dish& dish : dishes)
{
    showDishBrief(dish);
}
```

意思是：从 `dishes` 里一个一个取出菜品，每次把当前菜品叫做 `dish`。

`const Dish&` 表示只读引用，不复制菜品，也不修改菜品。

### 12.8 `size_t` 和 `static_cast<int>`

`vector.size()` 返回的类型通常是 `size_t`，它是无符号整数。函数要返回 `int`，所以用了：

```cpp
return static_cast<int>(i);
```

这表示显式把 `i` 转成 `int`。你暂时只需要知道：这是比 `(int)i` 更规范的 C++ 写法。

### 12.9 `setw`、`left`、`fixed`、`setprecision`

这些来自 `<iomanip>`，用于控制输出格式。

| 写法 | 意思 |
|---|---|
| `left` | 左对齐 |
| `setw(10)` | 占 10 个字符宽度 |
| `fixed` | 用普通小数格式输出 |
| `setprecision(2)` | 保留 2 位小数 |

这些语法主要是为了让控制台表格看起来更整齐。

### 12.10 三目运算符 `? :`

代码中：

```cpp
return status == OrderStatus::Reserved ? "已预订" : "已取餐";
```

意思是：

```text
如果 status 等于 Reserved，就返回“已预订”；否则返回“已取餐”。
```

可以看成简写版的 `if-else`。

---

## 13. 这个 demo 哪里适合你汇报

如果你要向老师解释这个 demo，可以重点讲这些点：

1. 学生信息用 `Student` 类表示。
2. 学生之间通过 `Student* next` 连接成链表。
3. `StudentList` 负责链表的增删查改。
4. 学生登录后通过 `student->placeOrder()`、`student->pickupOrder()` 调用成员函数。
5. 管理员通过全局函数管理学生、菜品和订单。
6. 菜品用 `vector<Dish>` 管理，订单用每个学生内部的 `vector<Order>` 管理。
7. 订餐成功后生成订单、扣减库存、生成取餐码。
8. 取餐时校验订单号和取餐码，并防止重复取餐。
9. 输入函数统一封装，避免非法输入导致程序崩掉。
10. 多级菜单通过函数嵌套调用实现。

---

## 14. 这个 demo 没有做什么

这也很重要，因为你要知道它只是题目 1 简化版，不是完整题目 2。

它目前没有完整实现：

- 真实支付：校园卡、微信、支付宝只是需求中提到，demo 没有真正接入。
- 退款处理：没有订单取消和退款流程。
- 文件存盘：程序关闭后数据不会自动保存。
- 食堂员工角色：demo 主要有管理员和学生。
- 班级管理：题目 2 扩展才要求。
- 评价留言和投诉建议：demo 中只展示评分，没有让学生新增评价。
- 每日/每周报表：demo 没有做统计报表。
- 图形界面：demo 是控制台界面。

所以如果后续做题目 2，就可以从这些方向扩展。

---

## 15. 读代码时最容易卡住的地方

### 15.1 为什么订单存在学生对象里

因为题目强调“学生通过自身对象调用成员函数实现订餐/取餐和查询功能”。订单放在学生内部，可以体现“这个学生管理自己的订单”。

管理员要查全部订单时，就遍历所有学生，再看每个学生的订单。

### 15.2 为什么菜品不用链表

题目说“菜品信息可使用结构体数组或链表单独定义”。所以用 `vector<Dish>` 是允许的。题目核心是学生类链表，不要求所有数据都用链表。

### 15.3 为什么有些函数传引用，有些传 const 引用

- 要修改外部数据：用普通引用，例如 `vector<Dish>& dishes`。
- 只读取外部数据：用 const 引用，例如 `const vector<Dish>& dishes`。
- 简单小数据，例如 `int`，可以直接传值。

### 15.4 为什么 `studentLogin` 返回 `Student*`

因为登录成功后，需要知道“到底是哪一个学生登录了”。返回学生对象指针后，菜单就可以调用：

```cpp
studentMenu(student, dishes, nextOrderId);
```

学生菜单里再通过：

```cpp
student->placeOrder(...);
student->showOrders();
```

操作这个具体学生。

---

## 16. 你可以怎样练习

建议你按下面步骤练：

1. 先运行 `project_bugfix_only.cpp`，体验菜单流程。
2. 登录管理员：`admin / 123456`。
3. 进入菜品管理，测试第 4 项“修改菜品”是否可用。
4. 登录学生：`20260001 / 111111`。
5. 订购一个菜品，记下取餐码。
6. 查询我的订单，观察订单状态是“已预订”。
7. 用取餐码取餐，再查订单，观察状态变成“已取餐”。
8. 再次取同一订单，观察“不能重复领取”的提示。
9. 管理员查询全部订单，观察刚刚学生订单能否被看到。
10. 打开逐行注释版，按照这个流程去代码里找对应函数。

---

## 17. 一句话总结整个项目

这个 demo 的本质是：

> 用学生类链表管理学生，用 vector 管理菜品，每个学生对象保存自己的订单；管理员通过全局函数管理全局数据，学生通过成员函数管理自己的订餐、取餐和查询流程。

掌握这个本质后，代码里的菜单、输入、输出、库存、订单状态，其实都是围绕这条主线展开的。
