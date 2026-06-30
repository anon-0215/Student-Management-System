/*
 * 校园学生食堂信息管理系统（题目 1 简化规范版）
 * ------------------------------------------------
 * 设计目标：
 * 1. 使用“学生类链表”保存学生信息，体现链表插入、删除、查找、遍历。
 * 2. 管理员作为超级用户，通过“全局函数”管理学生、菜品并查询订单。
 * 3. 学生登录后，通过“学生对象自己的成员函数”完成订餐、取餐、查询。
 * 4. 菜品使用结构体数组 vector 保存，代码难度适合大一学生阅读和汇报。
 *
 * 编码说明：本源码使用 UTF-8 编码保存。
 */
 
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits> 
#include <string>
#include <vector>

using namespace std;

// ========================= 常量与基础类型 =========================

const string ADMIN_NAME = "admin";
const string ADMIN_PASSWORD = "123456";
const int LOW_STOCK_LINE = 5;       // 库存低于等于该值时提示库存偏低
const int SCREEN_WIDTH = 72;        // 控制台分隔线宽度，仅用于美化显示

// 订单状态：
enum class OrderStatus //枚举类型，作用域调用，判断是否取餐
{
    Reserved,          // 已预订
    PickedUp           // 已取餐
};

// 菜品信息结构体：
struct Dish 
{
    int id;                 // 菜品编号，管理员和学生都通过编号选择菜品
    string name;            // 菜品名称
    string category;        // 分类：早餐、午餐、晚餐、特色菜等
    double price;           // 单价
    int stock;              // 当前库存数量
    string flavor;          // 口味说明
    string nutrition;       // 营养成分简述
    string allergen;        // 过敏源信息
    double rating;          // 平均评分
};

// 订单信息：每个学生对象内部保存自己的订单记录
struct Order 
{
    int orderId;            // 订单编号
    int dishId;             // 菜品编号
    string dishName;        // 下单时的菜品名，便于后续查询
    int quantity;           // 数量
    double amount;          // 金额
    string mealTime;        // 用餐时段，如 早餐 7:00-8:00
    string mode;            // 用餐方式：堂食 / 自提 / 外卖
    OrderStatus status;     // 订单状态，枚举二选一
    string pickupCode;      // 取餐码，用于防止重复取餐
};

// ========================= 工具函数 =========================
// 工具函数不属于某个学生对象，因此设计为全局函数


void printLine(char ch = '=') // 打印分割线函数，默认参数为 '='，如果传入其他字符，则打印该字符的分隔线
{
    cout << string(SCREEN_WIDTH, ch) << '\n';  //生成临时字符串对象，长度为 SCREEN_WIDTH，内容为 ch，语法为string(数量, 字符)
}

void pauseScreen()//暂停程序等待回车函数
{
    cout << "\n按回车键继续...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');//streamsize 这个类型能表示的最大值
    //cin.ignore()函数用于忽略输入缓冲区中的字符，直到遇到换行符为止
    //用于清空输入缓冲区，以便下一次输入时不会受到之前输入的影响
}

void clearInput() //清空输入缓冲区函数，把 cin 恢复正常，并把当前这一行剩下的输入全部清掉。
{
    cin.clear();//清除 cin 的错误状态，让cin重新变得可以读取。
    cin.ignore(numeric_limits<streamsize>::max(), '\n');//忽略输入缓冲区中剩余的错误输入，直到遇到换行符为止
}

int readInt(const string& prompt, int minValue, int maxValue)
 //读取整数输入函数，提示用户输入一个整数，并检查是否在指定范围内
 //如果输入无效，会提示用户重新输入，直到输入有效为止
 //eg:int choice = readInt("请选择：", 0, 3); 显示“请选择：”，然后要求用户输入一个 0 到 3 之间的整数。
{
    int value;
    while (true) 
    {
        cout << prompt;
        if (cin >> value && value >= minValue && value <= maxValue) 
        {
            clearInput();//把剩下的回车清掉，避免后面的 getline() 读取到空行
            return value;
        }
        cout << "输入无效，请输入 " << minValue << " 到 " << maxValue << " 之间的整数。\n";
        clearInput();//清除错误状态和输入缓冲区，准备下一次进入while循环
    }
}

double readDouble(const string& prompt, double minValue, double maxValue)//与readInt类似
 {
    double value;
    while (true) 
    {
        cout << prompt;
        if (cin >> value && value >= minValue && value <= maxValue) 
		{
            clearInput();
            return value;
        }
        cout << "输入无效，请输入 " << minValue << " 到 " << maxValue << " 之间的数字。\n";
        clearInput();
    }
}

string readText(const string& prompt, bool allowEmpty = false) 
//读取文本输入函数，提示用户输入一行文本，如果 allowEmpty 为 false，则要求输入非空文本
{
    string text;
    while (true) 
    {
        cout << prompt;
        getline(cin, text);
        if (allowEmpty || !text.empty())//如果允许为空，或者输入的文本不为空，则返回输入的文本
		 {
            return text;
        }
        cout << "内容不能为空，请重新输入。\n";
    }
}

string orderStatusToString(OrderStatus status)   //将订单状态枚举类型转换为中文
 {
    return status == OrderStatus::Reserved ? "已预订" : "已取餐"; 
    //三目，如果 status 是 Reserved，则返回 "已预订"，否则返回 "已取餐"
}

// 根据菜品编号查找菜品，返回 vector 下标；找不到返回 -1。
int findDishIndexById(const vector<Dish>& dishes, int dishId)
 {
    for (size_t i = 0; i < dishes.size(); ++i)//size_t 是无符号整数类型，适合表示容器大小
	 {
        if (dishes[i].id == dishId) 
		{
            return static_cast<int>(i);//把 size_t 类型的下标 i 转成 int，然后返回。
        }
    }
    return -1;
}

int getNextDishId(const vector<Dish>& dishes)//给新菜品自动生成下一个菜品编号
//管理员添加菜品时，会先调用它生成新编号，然后再输入菜品名称、价格、库存等信息
 {
    int maxId = 1000;//菜品编号是从 1001 开始的
    for (const Dish& dish : dishes) //语法糖，遍历向量dishes中的每个元素，dish是当前元素的引用
    {
        maxId = max(maxId, dish.id);//更新 maxId 为当前菜品编号和已有最大编号的较大值
    }
    return maxId + 1;
    //规避删除菜品后编号重复的问题，保证新菜品编号始终大于已有最大编号
}

string makePickupCode(const string& studentId, int orderId) 
{
    // 简单生成取餐码：学号后四位+订单号。
    string tail = studentId;
    if (tail.size() > 4) 
    {
        tail = tail.substr(tail.size() - 4);//从倒数第 4 个字符开始截取，一直截取到字符串结尾
    }
    return tail + "-" + to_string(orderId);//转字符串后拼接
}

void showDishTableHeader() // 打印菜品表头函数
{
    printLine('-');
    cout << left                      //输出左对齐
         << setw(8) << "编号"
         << setw(14) << "名称"
         << setw(10) << "分类"
         << setw(10) << "价格"
         << setw(8) << "库存"
         << setw(12) << "口味"
         << setw(8) << "评分" << '\n';
    printLine('-');
}

void showDishBrief(const Dish& dish) // 打印单个菜品简略信息函数
{
    cout << left
         << setw(8) << dish.id
         << setw(14) << dish.name
         << setw(10) << dish.category
         << setw(10) << fixed << setprecision(2) << dish.price
         << setw(8) << dish.stock
         << setw(12) << dish.flavor
         << setw(8) << fixed << setprecision(1) << dish.rating;
    if (dish.stock <= LOW_STOCK_LINE) //LOW_STOCK_LINE为全局常量，表示库存低于等于该值时提示库存偏低
    {
        cout << "  库存偏低";
    }
    cout << '\n';
}

void showDishDetail(const Dish& dish) 
{
    printLine('-');
    cout << "菜品编号：" << dish.id << '\n';
    cout << "菜品名称：" << dish.name << '\n';
    cout << "菜品分类：" << dish.category << '\n';
    cout << "单价：" << fixed << setprecision(2) << dish.price << " 元\n";
    cout << "库存：" << dish.stock << " 份\n";
    cout << "口味：" << dish.flavor << '\n';
    cout << "营养成分：" << dish.nutrition << '\n';
    cout << "过敏源：" << dish.allergen << '\n';
    cout << "评分：" << fixed << setprecision(1) << dish.rating << " 星\n";
    printLine('-');
}

void showAllDishes(const vector<Dish>& dishes) 
{
    if (dishes.empty()) {
        cout << "当前没有菜品信息。\n";
        return;
    }

    showDishTableHeader();
    for (const Dish& dish : dishes) 
	{
        showDishBrief(dish);
    }
    printLine('-');
}

// ========================= 学生类 =========================
// 学生类既保存学生基本信息，也保存该学生自己的订单。
// next 指针用于把多个学生对象连接成“学生类链表”。

class Student 
{
private:
    string studentId;            // 学号
    string password;             // 登录密码
    string name;                 // 姓名
    string gender;               // 性别
    string birthDate;            // 出生日期
    string grade;                // 年级
    string major;                // 专业
    string phone;                // 联系电话
    vector<Order> orders;        // 该学生自己的订单记录，Order 结构体类型向量

public:
    Student* next;               // 链表指针，指向下一个学生对象

    Student(const string& id,        // 构造函数
            const string& pwd,
            const string& stuName,
            const string& stuGender,
            const string& birthday,
            const string& stuGrade,
            const string& stuMajor,
            const string& stuPhone)
            
        : studentId(id),
          password(pwd),
          name(stuName),
          gender(stuGender),
          birthDate(birthday),
          grade(stuGrade),
          major(stuMajor),
          phone(stuPhone),
          next(nullptr) {}

    string getId() const { return studentId; }
    string getName() const { return name; }
    string getGrade() const { return grade; }
    string getMajor() const { return major; }
    const vector<Order>& getOrders() const { return orders; }

    bool checkPassword(const string& inputPassword) const 
	{
        return password == inputPassword;
    }

    void showBrief() const 
	{
        cout << left
             << setw(14) << studentId
             << setw(10) << name
             << setw(8) << gender
             << setw(10) << grade
             << setw(18) << major
             << setw(14) << phone << '\n';
    }

    void showProfile() const
	 {
        printLine('-');
        cout << "学号：" << studentId << '\n';
        cout << "姓名：" << name << '\n';
        cout << "性别：" << gender << '\n';
        cout << "出生日期：" << birthDate << '\n';
        cout << "年级：" << grade << '\n';
        cout << "专业：" << major << '\n';
        cout << "联系电话：" << phone << '\n';
        printLine('-');
    }

    void modifyByAdmin() {
        // 管理员修改学生信息时调用该成员函数。
        // 修改动作由管理员菜单触发，但具体数据仍属于这个学生对象本身。
        cout << "正在修改学生【" << name << "】的信息。直接回车表示保留原内容。\n";

        string newName = readText("姓名（当前：" + name + "）：", true);
        string newGender = readText("性别（当前：" + gender + "）：", true);
        string newBirthDate = readText("出生日期（当前：" + birthDate + "）：", true);
        string newGrade = readText("年级（当前：" + grade + "）：", true);
        string newMajor = readText("专业（当前：" + major + "）：", true);
        string newPhone = readText("联系电话（当前：" + phone + "）：", true);

        if (!newName.empty()) name = newName;
        if (!newGender.empty()) gender = newGender;
        if (!newBirthDate.empty()) birthDate = newBirthDate;
        if (!newGrade.empty()) grade = newGrade;
        if (!newMajor.empty()) major = newMajor;
        if (!newPhone.empty()) phone = newPhone;

        cout << "学生信息修改完成。\n";
    }

    void viewDishes(const vector<Dish>& dishes) const
	 {
        // 成员函数：学生查看菜品。
        // 虽然菜品列表是外部数据，但“查看”这个动作是当前学生对象发起的。
        cout << "\n【" << name << "】正在查看今日菜单\n";
        showAllDishes(dishes);

        int choice = readInt("是否查看某个菜品详情？1.是  0.否：", 0, 1);
        if (choice == 1) {
            int dishId = readInt("请输入菜品编号：", 1, 999999);
            int index = findDishIndexById(dishes, dishId);
            if (index == -1) {
                cout << "没有找到该菜品。\n";
            } else {
                showDishDetail(dishes[index]);
            }
        }
    }

    void placeOrder(vector<Dish>& dishes, int& nextOrderId)
	 {
        // 成员函数：当前学生对象为自己订餐。
        // 订单会被保存到 this->orders 中，这正是成员函数适合做的事情。
        if (dishes.empty()) {
            cout << "当前没有可订购的菜品。\n";
            return;
        }

        showAllDishes(dishes);
        int dishId = readInt("请输入要订购的菜品编号：", 1, 999999);
        int index = findDishIndexById(dishes, dishId);
        if (index == -1) {
            cout << "菜品编号不存在，订餐失败。\n";
            return;
        }

        Dish& selectedDish = dishes[index];
        if (selectedDish.stock <= 0) {
            cout << "该菜品已经售罄，订餐失败。\n";
            return;
        }

        int quantity = readInt("请输入订购数量：", 1, selectedDish.stock);

        cout << "\n请选择用餐时段：\n";
        cout << "1. 早餐 7:00-8:00\n";
        cout << "2. 午餐 11:30-12:30\n";
        cout << "3. 晚餐 17:30-18:30\n";
        int timeChoice = readInt("请选择：", 1, 3);

        string mealTime;
        if (timeChoice == 1) mealTime = "早餐 7:00-8:00";
        if (timeChoice == 2) mealTime = "午餐 11:30-12:30";
        if (timeChoice == 3) mealTime = "晚餐 17:30-18:30";

        cout << "\n请选择用餐方式：\n";
        cout << "1. 堂食\n";
        cout << "2. 自提\n";
        cout << "3. 外卖\n";
        int modeChoice = readInt("请选择：", 1, 3);

        string mode;
        if (modeChoice == 1) mode = "堂食";
        if (modeChoice == 2) mode = "自提";
        if (modeChoice == 3) mode = "外卖";

        Order newOrder;
        newOrder.orderId = nextOrderId++;
        newOrder.dishId = selectedDish.id;
        newOrder.dishName = selectedDish.name;
        newOrder.quantity = quantity;
        newOrder.amount = selectedDish.price * quantity;
        newOrder.mealTime = mealTime;
        newOrder.mode = mode;
        newOrder.status = OrderStatus::Reserved;
        newOrder.pickupCode = makePickupCode(studentId, newOrder.orderId);

        orders.push_back(newOrder);
        selectedDish.stock -= quantity;

        cout << "\n订餐成功！\n";
        cout << "订单编号：" << newOrder.orderId << '\n';
        cout << "菜品：" << newOrder.dishName << " × " << newOrder.quantity << '\n';
        cout << "金额：" << fixed << setprecision(2) << newOrder.amount << " 元\n";
        cout << "取餐码：" << newOrder.pickupCode << '\n';
        cout << "请妥善保存取餐码，取餐时需要验证。\n";
    }

    void pickupOrder() {
        // 成员函数：当前学生对象领取自己的订单。
        // 取餐成功后，订单状态从“已预订”变为“已取餐”，防止重复领取。
        if (orders.empty()) {
            cout << "你目前没有订单。\n";
            return;
        }

        showOrders();
        int orderId = readInt("请输入要取餐的订单编号：", 1, 999999);
        string code = readText("请输入取餐码：");

        for (Order& order : orders) {
            if (order.orderId == orderId) {
                if (order.status == OrderStatus::PickedUp) {
                    cout << "该订单已经取过餐，不能重复领取。\n";
                    return;
                }
                if (order.pickupCode != code) {
                    cout << "取餐码错误，验证失败。\n";
                    return;
                }

                order.status = OrderStatus::PickedUp;
                cout << "取餐成功，订单状态已更新为【已取餐】。\n";
                return;
            }
        }

        cout << "没有找到该订单。\n";
    }

    void showOrders() const {
        // 成员函数：只显示当前学生自己的订单。
        if (orders.empty()) {
            cout << "当前没有订单记录。\n";
            return;
        }

        printLine('-');
        cout << left
             << setw(10) << "订单号"
             << setw(14) << "菜品"
             << setw(8) << "数量"
             << setw(10) << "金额"
             << setw(18) << "时段"
             << setw(8) << "方式"
             << setw(10) << "状态" << '\n';
        printLine('-');

        for (const Order& order : orders) {
            cout << left
                 << setw(10) << order.orderId
                 << setw(14) << order.dishName
                 << setw(8) << order.quantity
                 << setw(10) << fixed << setprecision(2) << order.amount
                 << setw(18) << order.mealTime
                 << setw(8) << order.mode
                 << setw(10) << orderStatusToString(order.status) << '\n';
        }
        printLine('-');
    }
};

// ========================= 学生链表类 =========================
// StudentList 负责管理链表本身：头指针、插入、删除、查找、遍历。
// 这样比把所有链表代码写在 main 里更清楚，也方便汇报。

class StudentList 
{
private:
    Student* head;

public:
    StudentList() : head(nullptr) {}

    ~StudentList() 
	{
        clear();
    }

    Student* getHead() const 
	{
        return head;
    }

    bool isEmpty() const 
	{
        return head == nullptr;
    }

    void clear() 
	{
        Student* current = head;
        while (current != nullptr) 
		{
            Student* nextNode = current->next;
            delete current;
            current = nextNode;
        }
        head = nullptr;
    }

    Student* findById(const string& studentId) const 
	{
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
    }

    bool addStudent(Student* newStudent) 
	{
        if (newStudent == nullptr) 
		{
            return false;
        }
        if (findById(newStudent->getId()) != nullptr) 
		{
            return false;
        }

        // 头插法：代码短，效率高，适合链表基础训练。
        newStudent->next = head;
        head = newStudent;
        return true;
    }

    bool removeById(const string& studentId) {
        Student* current = head;
        Student* previous = nullptr;

        while (current != nullptr) {
            if (current->getId() == studentId) {
                if (previous == nullptr) {
                    head = current->next;
                } else {
                    previous->next = current->next;
                }
                delete current;
                return true;
            }
            previous = current;
            current = current->next;
        }
        return false;
    }

    void showAllStudents() const {
        if (isEmpty()) {
            cout << "当前没有学生信息。\n";
            return;
        }

        printLine('-');
        cout << left
             << setw(14) << "学号"
             << setw(10) << "姓名"
             << setw(8) << "性别"
             << setw(10) << "年级"
             << setw(18) << "专业"
             << setw(14) << "联系电话" << '\n';
        printLine('-');

        Student* current = head;
        while (current != nullptr) {
            current->showBrief();
            current = current->next;
        }
        printLine('-');
    }

    int countStudents() const {
        int count = 0;
        Student* current = head;
        while (current != nullptr) {
            ++count;
            current = current->next;
        }
        return count;
    }
};

// ========================= 初始化数据 =========================

void initStudents(StudentList& students) {
    students.addStudent(new Student("20260001", "111111", "李明", "男", "2007-03-12", "大一", "软件工程", "13800000001"));
    students.addStudent(new Student("20260002", "222222", "王雨", "女", "2007-07-05", "大一", "计算机科学", "13800000002"));
    students.addStudent(new Student("20260003", "333333", "张辰", "男", "2006-11-20", "大二", "网络工程", "13800000003"));
}

void initDishes(vector<Dish>& dishes) {
    dishes.push_back({1001, "鸡蛋灌饼", "早餐", 6.50, 30, "咸香", "蛋白质、碳水", "鸡蛋、面粉", 4.6});
    dishes.push_back({1002, "小米粥", "早餐", 3.00, 40, "清淡", "碳水、膳食纤维", "无明显过敏源", 4.4});
    dishes.push_back({1003, "红烧鸡腿", "午餐", 12.00, 25, "咸鲜", "蛋白质、脂肪", "大豆", 4.7});
    dishes.push_back({1004, "番茄炒蛋", "午餐", 8.00, 20, "酸甜", "维生素、蛋白质", "鸡蛋", 4.5});
    dishes.push_back({1005, "麻辣香锅", "晚餐", 18.00, 15, "麻辣", "蛋白质、脂肪", "花生、芝麻", 4.8});
    dishes.push_back({1006, "素炒西兰花", "特色菜", 7.00, 18, "清爽", "维生素、膳食纤维", "无明显过敏源", 4.3});
}

// ========================= 管理员全局函数：学生管理 =========================
// 这些函数没有 this 指针，不属于某一个学生对象。
// 它们代表管理员从系统整体角度进行管理，符合题目对“全局函数”的要求。

Student* createStudentFromInput() {
    string id = readText("请输入学号：");
    string password = readText("请输入初始密码：");
    string name = readText("请输入姓名：");
    string gender = readText("请输入性别：");
    string birthDate = readText("请输入出生日期：");
    string grade = readText("请输入年级：");
    string major = readText("请输入专业：");
    string phone = readText("请输入联系电话：");

    return new Student(id, password, name, gender, birthDate, grade, major, phone);
}

void adminAddStudent(StudentList& students) {
    Student* newStudent = createStudentFromInput();
    if (students.addStudent(newStudent)) {
        cout << "学生添加成功。\n";
    } else {
        cout << "添加失败：学号可能已经存在。\n";
        delete newStudent;
    }
}

void adminDeleteStudent(StudentList& students) {
    string id = readText("请输入要删除的学生学号：");
    Student* student = students.findById(id);
    if (student == nullptr) {
        cout << "没有找到该学生。\n";
        return;
    }

    cout << "即将删除学生：" << student->getName() << "（" << student->getId() << "）\n";
    int confirm = readInt("确认删除？1.确认  0.取消：", 0, 1);
    if (confirm == 1 && students.removeById(id)) {
        cout << "删除成功。\n";
    } else {
        cout << "已取消删除。\n";
    }
}

void adminSearchStudent(const StudentList& students) {
    string id = readText("请输入要查询的学生学号：");
    Student* student = students.findById(id);
    if (student == nullptr) {
        cout << "没有找到该学生。\n";
        return;
    }

    student->showProfile();
    cout << "该学生订单：\n";
    student->showOrders();
}

void adminModifyStudent(StudentList& students) {
    string id = readText("请输入要修改的学生学号：");
    Student* student = students.findById(id);
    if (student == nullptr) {
        cout << "没有找到该学生。\n";
        return;
    }

    student->modifyByAdmin();
}

void studentManageMenu(StudentList& students) {
    while (true) {
        printLine();
        cout << "管理员菜单 > 学生数据管理\n";
        printLine();
        cout << "1. 查看全部学生\n";
        cout << "2. 增加学生\n";
        cout << "3. 删除学生\n";
        cout << "4. 修改学生信息\n";
        cout << "5. 查询学生及其订单\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 5);
        if (choice == 0) return;

        switch (choice) {
            case 1:
                students.showAllStudents();
                cout << "学生总人数：" << students.countStudents() << '\n';
                break;
            case 2:
                adminAddStudent(students);
                break;
            case 3:
                adminDeleteStudent(students);
                break;
            case 4:
                adminModifyStudent(students);
                break;
            case 5:
                adminSearchStudent(students);
                break;
        }
        pauseScreen();
    }
}

// ========================= 管理员全局函数：菜品管理 =========================

void adminAddDish(vector<Dish>& dishes) {
    Dish dish;
    dish.id = getNextDishId(dishes);
    cout << "新菜品编号自动生成：" << dish.id << '\n';

    dish.name = readText("请输入菜品名称：");
    dish.category = readText("请输入菜品分类：");
    dish.price = readDouble("请输入价格：", 0.1, 9999.0);
    dish.stock = readInt("请输入库存数量：", 0, 99999);
    dish.flavor = readText("请输入口味：");
    dish.nutrition = readText("请输入营养成分：");
    dish.allergen = readText("请输入过敏源信息：");
    dish.rating = readDouble("请输入评分（1-5）：", 1.0, 5.0);

    dishes.push_back(dish);
    cout << "菜品添加成功。\n";
}

void adminDeleteDish(vector<Dish>& dishes) {
    showAllDishes(dishes);
    int dishId = readInt("请输入要删除的菜品编号：", 1, 999999);
    int index = findDishIndexById(dishes, dishId);
    if (index == -1) {
        cout << "没有找到该菜品。\n";
        return;
    }

    cout << "即将删除菜品：" << dishes[index].name << '\n';
    int confirm = readInt("确认删除？1.确认  0.取消：", 0, 1);
    if (confirm == 1) {
        dishes.erase(dishes.begin() + index);
        cout << "删除成功。\n";
    } else {
        cout << "已取消删除。\n";
    }
}

void adminModifyDish(vector<Dish>& dishes) {
    showAllDishes(dishes);
    int dishId = readInt("请输入要修改的菜品编号：", 1, 999999);
    int index = findDishIndexById(dishes, dishId);
    if (index == -1) {
        cout << "没有找到该菜品。\n";
        return;
    }

    Dish& dish = dishes[index];
    cout << "正在修改菜品【" << dish.name << "】。直接回车表示保留文字信息。\n";

    string newName = readText("菜品名称（当前：" + dish.name + "）：", true);
    string newCategory = readText("分类（当前：" + dish.category + "）：", true);
    string newFlavor = readText("口味（当前：" + dish.flavor + "）：", true);
    string newNutrition = readText("营养成分（当前：" + dish.nutrition + "）：", true);
    string newAllergen = readText("过敏源（当前：" + dish.allergen + "）：", true);

    if (!newName.empty()) dish.name = newName;
    if (!newCategory.empty()) dish.category = newCategory;
    if (!newFlavor.empty()) dish.flavor = newFlavor;
    if (!newNutrition.empty()) dish.nutrition = newNutrition;
    if (!newAllergen.empty()) dish.allergen = newAllergen;

    int changeNumber = readInt("是否修改价格、库存、评分？1.是  0.否：", 0, 1);
    if (changeNumber == 1) {
        dish.price = readDouble("请输入新价格：", 0.1, 9999.0);
        dish.stock = readInt("请输入新库存：", 0, 99999);
        dish.rating = readDouble("请输入新评分（1-5）：", 1.0, 5.0);
    }

    cout << "菜品信息修改完成。\n";
}

void dishManageMenu(vector<Dish>& dishes) {
    while (true) {
        printLine();
        cout << "管理员菜单 > 菜品管理\n";
        printLine();
        cout << "1. 查看全部菜品\n";
        cout << "2. 增加菜品\n";
        cout << "3. 删除菜品\n";
        cout << "4. 修改菜品\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 4);
        if (choice == 0) return;

        switch (choice) {
            case 1:
                showAllDishes(dishes);
                break;
            case 2:
                adminAddDish(dishes);
                break;
            case 3:
                adminDeleteDish(dishes);
                break;
            case 4:
                adminModifyDish(dishes);
                break;
        }
        pauseScreen();
    }
}

// ========================= 管理员全局函数：订单查询 =========================

void adminShowAllOrders(const StudentList& students) 
{
    Student* current = students.getHead();
    bool hasOrder = false;

    printLine('-');
    cout << left
         << setw(12) << "学生"
         << setw(12) << "学号"
         << setw(10) << "订单号"
         << setw(14) << "菜品"
         << setw(8) << "数量"
         << setw(10) << "金额"
         << setw(10) << "状态" << '\n';
    printLine('-');

    while (current != nullptr) 
	{
        const vector<Order>& orders = current->getOrders();
        for (const Order& order : orders) 
		{
            hasOrder = true;
            cout << left
                 << setw(12) << current->getName()
                 << setw(12) << current->getId()
                 << setw(10) << order.orderId
                 << setw(14) << order.dishName
                 << setw(8) << order.quantity
                 << setw(10) << fixed << setprecision(2) << order.amount
                 << setw(10) << orderStatusToString(order.status) << '\n';
        }
        current = current->next;
    }

    if (!hasOrder)
	{
        cout << "当前还没有任何订单。\n";
    }
    printLine('-');
}

void adminSearchOrdersByDish(const StudentList& students) 
{
    string keyword = readText("请输入要查询的菜品名称关键字：");
    Student* current = students.getHead();
    bool found = false;

    printLine('-');
    cout << "订购包含【" << keyword << "】的菜品的学生：\n";
    printLine('-');

    while (current != nullptr) {
        const vector<Order>& orders = current->getOrders();
        for (const Order& order : orders) {
            if (order.dishName.find(keyword) != string::npos) 
			{
                found = true;
                cout << "学生：" << current->getName()
                     << "，学号：" << current->getId()
                     << "，订单号：" << order.orderId
                     << "，菜品：" << order.dishName
                     << "，数量：" << order.quantity
                     << "，状态：" << orderStatusToString(order.status) << '\n';
            }
        }
        current = current->next;
    }

    if (!found) {
        cout << "没有查询到相关订单。\n";
    }
}

void adminSearchOrdersByStudent(const StudentList& students) {
    string id = readText("请输入学生学号：");
    Student* student = students.findById(id);
    if (student == nullptr) {
        cout << "没有找到该学生。\n";
        return;
    }

    cout << "学生【" << student->getName() << "】的订单信息：\n";
    student->showOrders();
}

void orderQueryMenu(const StudentList& students) {
    while (true) {
        printLine();
        cout << "管理员菜单 > 订单查询\n";
        printLine();
        cout << "1. 查看全部订单\n";
        cout << "2. 按学生查询订单\n";
        cout << "3. 按菜品名称查询订单\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 3);
        if (choice == 0) return;

        switch (choice) {
            case 1:
                adminShowAllOrders(students);
                break;
            case 2:
                adminSearchOrdersByStudent(students);
                break;
            case 3:
                adminSearchOrdersByDish(students);
                break;
        }
        pauseScreen();
    }
}

// ========================= 登录与主菜单 =========================

bool adminLogin() {
    string name = readText("管理员账号：");
    string password = readText("管理员密码：");

    if (name == ADMIN_NAME && password == ADMIN_PASSWORD) {
        cout << "管理员登录成功。\n";
        return true;
    }
    cout << "账号或密码错误。\n";
    return false;
}

Student* studentLogin(StudentList& students) {
    string id = readText("请输入学号：");
    string password = readText("请输入密码：");

    Student* student = students.findById(id);
    if (student == nullptr || !student->checkPassword(password)) {
        cout << "学号或密码错误。\n";
        return nullptr;
    }

    cout << "学生登录成功，欢迎你，" << student->getName() << "！\n";
    return student;
}

void registerStudent(StudentList& students) {
    cout << "\n【学生注册】\n";
    Student* newStudent = createStudentFromInput();
    if (students.addStudent(newStudent)) {
        cout << "注册成功，请返回主菜单登录。\n";
    } else {
        cout << "注册失败：该学号已经存在。\n";
        delete newStudent;
    }
}

void adminMenu(StudentList& students, vector<Dish>& dishes) {
    while (true) {
        printLine();
        cout << "校园食堂信息管理系统 > 管理员菜单\n";
        printLine();
        cout << "1. 学生数据管理\n";
        cout << "2. 菜品管理\n";
        cout << "3. 订单查询\n";
        cout << "0. 退出管理员菜单\n";

        int choice = readInt("请选择：", 0, 3);
        if (choice == 0) return;

        switch (choice) {
            case 1:
                studentManageMenu(students);
                break;
            case 2:
                dishManageMenu(dishes);
                break;
            case 3:
                orderQueryMenu(students);
                break;
        }
    }
}

void studentMenu(Student* student, vector<Dish>& dishes, int& nextOrderId) {
    // 注意：这里 student 是指针，下面通过 student->成员函数 调用。
    // 这能直接体现题目要求中的“通过指针访问成员函数”和面向对象思想。
    while (true) {
        printLine();
        cout << "校园食堂信息管理系统 > 学生菜单\n";
        printLine();
        cout << "当前用户：" << student->getName() << "（" << student->getId() << "）\n";
        cout << "1. 查看菜品\n";
        cout << "2. 在线订餐\n";
        cout << "3. 取餐验证\n";
        cout << "4. 查询我的订单\n";
        cout << "5. 查看个人信息\n";
        cout << "0. 退出学生菜单\n";

        int choice = readInt("请选择：", 0, 5);
        if (choice == 0) return;

        switch (choice) {
            case 1:
                student->viewDishes(dishes);
                break;
            case 2:
                student->placeOrder(dishes, nextOrderId);
                break;
            case 3:
                student->pickupOrder();
                break;
            case 4:
                student->showOrders();
                break;
            case 5:
                student->showProfile();
                break;
        }
        pauseScreen();
    }
}

void mainMenu(StudentList& students, vector<Dish>& dishes, int& nextOrderId) 
{
    while (true) 
	{
        printLine();
        cout << "校园学生食堂信息管理系统\n";
        printLine();
        cout << "1. 管理员登录\n";
        cout << "2. 学生登录\n";
        cout << "3. 学生注册\n";
        cout << "0. 退出系统\n";
        printLine();
        cout << "测试管理员：admin / 123456\n";
        cout << "测试学生：20260001 / 111111\n";

        int choice = readInt("请选择：", 0, 3);
        if (choice == 0) {
            cout << "感谢使用校园食堂信息管理系统，再见！\n";
            return;
        }

        switch (choice) {
            case 1:
                if (adminLogin()) {
                    adminMenu(students, dishes);
                }
                pauseScreen();
                break;
            case 2: {
                Student* student = studentLogin(students);
                if (student != nullptr) {
                    studentMenu(student, dishes, nextOrderId);
                }
                pauseScreen();
                break;
            }
            case 3:
                registerStudent(students);
                pauseScreen();
                break;
        }
    }
}

// ========================= 主函数 =========================

int main()
{


    StudentList students;
    vector<Dish> dishes;
    int nextOrderId = 1;
    initStudents(students);
    initDishes(dishes);
    mainMenu(students, dishes, nextOrderId);
    return 0;
}

