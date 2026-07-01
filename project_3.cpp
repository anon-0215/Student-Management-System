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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits> 
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ========================= 常量与基础类型 =========================

const string ADMIN_NAME = "admin";
const string ADMIN_PASSWORD = "123456";
const int LOW_STOCK_LINE = 5;       // 库存低于等于该值时提示库存偏低
const int SCREEN_WIDTH = 72;        // 控制台分隔线宽度，仅用于美化显示

// 文件名常量：文件会保存在程序运行目录下，用于实现自动存盘。
const string STUDENT_FILE = "students.txt";
const string DISH_FILE = "dishes.txt";
const string ORDER_FILE = "orders.txt";
const string CLASS_FILE = "classes.txt";
const string COMPLAINT_FILE = "complaints.txt";
const string COUNTER_FILE = "counters.txt";

// 订单状态：
enum class OrderStatus //枚举类型，作用域调用，判断订单处于哪一个业务阶段
{
    Reserved,          // 已预订，等待支付
    Paid,              // 已支付，等待取餐
    PickedUp,          // 已取餐
    Cancelled,         // 已取消，未支付订单取消
    Refunded           // 已退款，已支付订单取消后退款
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
    int ratingCount;        // 参与评分的人数，用于动态计算平均分
    vector<string> comments;// 学生评价留言，方便管理员查看反馈
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
    OrderStatus status;     // 订单状态，枚举类型
    string pickupCode;      // 取餐码，用于防止重复取餐
    string payMethod;       // 支付方式：未支付 / 校园卡 / 微信 / 支付宝
};

// 投诉建议信息：学生提交，管理员回复处理。
struct Complaint
{
    int complaintId;        // 投诉建议编号
    string studentId;       // 提交学生学号
    string studentName;     // 提交学生姓名
    string type;            // 类型：卫生 / 服务 / 菜品 / 其他
    string content;         // 具体内容
    string reply;           // 管理员回复
    bool handled;           // 是否已处理
};

// 班级信息：题目 2 要求引入班级信息，这里用 vector 保存班级表。
struct ClassInfo
{
    string className;       // 班级名称，例如 软件一班
    string grade;           // 年级
    string major;           // 专业
    string advisor;         // 辅导员或班主任
    string remark;          // 备注
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
    switch (status)
    {
        case OrderStatus::Reserved: return "待支付";
        case OrderStatus::Paid: return "已支付";
        case OrderStatus::PickedUp: return "已取餐";
        case OrderStatus::Cancelled: return "已取消";
        case OrderStatus::Refunded: return "已退款";
    }
    return "未知";
}

int orderStatusToInt(OrderStatus status) //把订单状态转换为整数，方便保存到文件
{
    switch (status)
    {
        case OrderStatus::Reserved: return 0;
        case OrderStatus::Paid: return 1;
        case OrderStatus::PickedUp: return 2;
        case OrderStatus::Cancelled: return 3;
        case OrderStatus::Refunded: return 4;
    }
    return 0;
}

OrderStatus intToOrderStatus(int value) //从文件读取整数后，还原成订单状态枚举
{
    if (value == 1) return OrderStatus::Paid;
    if (value == 2) return OrderStatus::PickedUp;
    if (value == 3) return OrderStatus::Cancelled;
    if (value == 4) return OrderStatus::Refunded;
    return OrderStatus::Reserved;
}

vector<string> splitLine(const string& line, char delimiter) //按指定分隔符拆分字符串，用于读文件
{
    vector<string> result;
    string item;
    stringstream ss(line);
    while (getline(ss, item, delimiter))
    {
        result.push_back(item);
    }
    return result;
}

string joinStrings(const vector<string>& data, const string& delimiter) //把字符串向量拼接成一行，用于存文件
{
    string result;
    for (size_t i = 0; i < data.size(); ++i)
    {
        if (i > 0) result += delimiter;
        result += data[i];
    }
    return result;
}

bool shouldCountRevenue(OrderStatus status) //统计营收时，只统计已经支付或已经取餐的订单
{
    return status == OrderStatus::Paid || status == OrderStatus::PickedUp;
}

void showDataFileHint() //提示数据文件保存位置，避免误以为文件会生成在源码旁边
{
    cout << "\n【数据存档说明】\n";
    cout << "本程序会把数据保存为 students.txt、dishes.txt、orders.txt、classes.txt、complaints.txt、counters.txt。\n";
    cout << "这些文件生成在程序运行目录下，也就是终端当前所在目录，不一定是 cpp 源码所在目录。\n";
    cout << "正常退出学生菜单、管理员菜单或退出系统后会自动保存一次数据。\n";
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

void showDishDetail(const Dish& dish) //打印 单个 菜品 详细 信息函数
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
    cout << "评分：" << fixed << setprecision(1) << dish.rating << " 星";
    cout << "（" << dish.ratingCount << " 人评分）\n";
    if (!dish.comments.empty())
    {
        cout << "学生留言：\n";
        for (size_t i = 0; i < dish.comments.size(); ++i)
        {
            cout << "  " << i + 1 << ". " << dish.comments[i] << '\n';
        }
    }
    printLine('-');
}

void showAllDishes(const vector<Dish>& dishes) //打印 所有 菜品 简略 信息函数
{
    if (dishes.empty()) 
    {
        cout << "当前没有菜品信息。\n";
        return;
    }

    // 说明：中文菜名长度不固定，用表格显示时在不同终端容易错位。
    // 这里改成一行一个菜品，既清楚又方便大一阶段汇报演示。
    printLine('-');
    cout << "今日菜单：\n";
    for (const Dish& dish : dishes) //遍历菜品向量，逐个打印菜品简略信息
	{
        cout << "[" << dish.id << "] " << dish.name
             << " | " << dish.category
             << " | " << fixed << setprecision(2) << dish.price << " 元"
             << " | 库存 " << dish.stock
             << " | 口味 " << dish.flavor
             << " | 评分 " << fixed << setprecision(1) << dish.rating;
        if (dish.stock <= LOW_STOCK_LINE) //LOW_STOCK_LINE为全局常量，表示库存低于等于该值时提示库存偏低
        {
            cout << " | 库存偏低";
        }
        cout << '\n';
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
    string className;            // 班级
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
            const string& stuPhone,
            const string& stuClassName = "未分班")
            
        : studentId(id),
          password(pwd),
          name(stuName),
          gender(stuGender),
          birthDate(birthday),
          grade(stuGrade),
          major(stuMajor),
          className(stuClassName),
          phone(stuPhone),
          next(nullptr) {}

    string getId() const { return studentId; } //const保护对象成员
    string getPasswordForFile() const { return password; }
    string getName() const { return name; }
    string getGender() const { return gender; }
    string getBirthDate() const { return birthDate; }
    string getGrade() const { return grade; }
    string getMajor() const { return major; }
    string getClassName() const { return className; }
    string getPhone() const { return phone; }
    const vector<Order>& getOrders() const { return orders; } // 返回订单向量的常量引用，避免拷贝，提高效率

    void addOrderFromFile(const Order& order) //从文件恢复订单时使用，仍然把订单保存到学生对象内部
    {
        orders.push_back(order);
    }

    bool checkPassword(const string& inputPassword) const 
	{
        return password == inputPassword;
    }

    void showBrief() const    //打印学生简略信息函数
	{
        cout << left
             << setw(14) << studentId
             << setw(10) << name
             << setw(8) << gender
             << setw(10) << grade
             << setw(18) << major
             << setw(14) << className
             << setw(14) << phone << '\n';
    }

    void showProfile() const   //打印学生详细信息函数
	 {
        printLine('-');
        cout << "学号：" << studentId << '\n';
        cout << "姓名：" << name << '\n';
        cout << "性别：" << gender << '\n';
        cout << "出生日期：" << birthDate << '\n';
        cout << "年级：" << grade << '\n';
        cout << "专业：" << major << '\n';
        cout << "班级：" << className << '\n';
        cout << "联系电话：" << phone << '\n';
        printLine('-');
    }

    void modifyByAdmin() 
    {
        // 管理员修改学生信息时调用该成员函数,可以修改学生私有成员
        // 修改动作由管理员菜单触发，但具体数据仍属于这个学生对象本身。
        cout << "正在修改学生【" << name << "】的信息。直接回车表示保留原内容。\n";

        string newName = readText("姓名（当前：" + name + "）：", true); //字符串拼接，且允许为空
        string newGender = readText("性别（当前：" + gender + "）：", true);
        string newBirthDate = readText("出生日期（当前：" + birthDate + "）：", true);
        string newGrade = readText("年级（当前：" + grade + "）：", true);
        string newMajor = readText("专业（当前：" + major + "）：", true);
        string newClassName = readText("班级（当前：" + className + "）：", true);
        string newPhone = readText("联系电话（当前：" + phone + "）：", true);

        if (!newName.empty()) name = newName; //如果输入不为空，则更新学生信息
        if (!newGender.empty()) gender = newGender;
        if (!newBirthDate.empty()) birthDate = newBirthDate;
        if (!newGrade.empty()) grade = newGrade;
        if (!newMajor.empty()) major = newMajor;
        if (!newClassName.empty()) className = newClassName;
        if (!newPhone.empty()) phone = newPhone;

        cout << "学生信息修改完成。\n";
    }

    void viewDishes(const vector<Dish>& dishes) const//打印所有菜品简略信息，并可选择查看某个菜品详细信息
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

    void placeOrder(vector<Dish>& dishes, int& nextOrderId)//下单函数。第二个参数在主函数中定义，初始值为 1，每次下单后自增
	 {
        // 成员函数：当前学生对象为自己订餐。
        // 订单会被保存到 this->orders 中，这正是成员函数适合做的事情。
        if (dishes.empty()) 
        {
            cout << "当前没有可订购的菜品。\n";
            return;
        }

        showAllDishes(dishes);////打印所有菜品简略信息
        int dishId = readInt("请输入要订购的菜品编号：", 1, 999999);
        int index = findDishIndexById(dishes, dishId);// 根据菜品编号查找菜品，返回 vector 下标；找不到返回 -1。
        if (index == -1) 
        {
            cout << "菜品编号不存在，订餐失败。\n";
            return;
        }

        Dish& selectedDish = dishes[index];//所选菜品的别名，方便后续操作
        if (selectedDish.stock <= 0) 
        {
            cout << "该菜品已经售罄，订餐失败。\n";
            return;
        }

        int quantity = readInt("请输入订购数量：", 1, selectedDish.stock);//读取订购数量，范围是 1 到当前库存

        cout << "\n请选择用餐时段：\n";
        cout << "1. 早餐 7:00-8:00\n";
        cout << "2. 午餐 11:30-12:30\n";
        cout << "3. 晚餐 17:30-18:30\n";
        int timeChoice = readInt("请选择：", 1, 3);//读取用餐时段选择，范围是 1 到 3

        string mealTime;//用餐时段字符串
        if (timeChoice == 1) mealTime = "早餐 7:00-8:00";
        if (timeChoice == 2) mealTime = "午餐 11:30-12:30";
        if (timeChoice == 3) mealTime = "晚餐 17:30-18:30";

        cout << "\n请选择用餐方式：\n";
        cout << "1. 堂食\n";
        cout << "2. 自提\n";
        cout << "3. 外卖\n";
        int modeChoice = readInt("请选择：", 1, 3);//读取用餐方式选择，范围是 1 到 3

        string mode;//用餐方式字符串
        if (modeChoice == 1) mode = "堂食";
        if (modeChoice == 2) mode = "自提";
        if (modeChoice == 3) mode = "外卖";

        Order newOrder;//创建新订单对象
        newOrder.orderId = nextOrderId++;//先使用当前订单编号，然后自增，为下一个订单准备
        newOrder.dishId = selectedDish.id;
        newOrder.dishName = selectedDish.name;
        newOrder.quantity = quantity;
        newOrder.amount = selectedDish.price * quantity;
        newOrder.mealTime = mealTime;
        newOrder.mode = mode;
        newOrder.status = OrderStatus::Reserved;//新订单状态为待支付
        newOrder.pickupCode = makePickupCode(studentId, newOrder.orderId);//生成取餐码
        newOrder.payMethod = "未支付";//刚下单时还没有完成支付

        orders.push_back(newOrder);//将新订单添加到当前学生对象的订单向量中
        selectedDish.stock -= quantity;

        cout << "\n订餐成功！\n";
        cout << "订单编号：" << newOrder.orderId << '\n';//显示订单编号
        cout << "菜品：" << newOrder.dishName << " × " << newOrder.quantity << '\n';//显示菜品名称和数量
        cout << "金额：" << fixed << setprecision(2) << newOrder.amount << " 元\n";//显示金额，保留两位小数
        cout << "订单状态：待支付\n";
        cout << "取餐码：" << newOrder.pickupCode << '\n';//显示取餐码
        cout << "请先完成支付，再凭取餐码取餐。\n";//显示取餐提示
    }

    void pickupOrder() //取餐函数
    {
        // 成员函数：当前学生对象领取自己的订单。
        // 取餐成功后，订单状态从“已预订”变为“已取餐”，防止重复领取。
        if (orders.empty()) //如果当前学生对象的订单向量为空，则提示没有订单
        {
            cout << "你目前没有订单。\n";
            return;
        }

        showOrders();//打印当前学生对象的所有订单简略信息
        int orderId = readInt("请输入要取餐的订单编号：", 1, 999999);
        string code = readText("请输入取餐码：");//读取取餐码输入，不允许为空

        for (Order& order : orders) 
        {
            if (order.orderId == orderId) //找到对应订单
            {
                if (order.status == OrderStatus::PickedUp) 
                {
                    cout << "该订单已经取过餐，不能重复领取。\n";
                    return;
                }
                if (order.status == OrderStatus::Reserved)
                {
                    cout << "该订单还没有支付，请先完成支付。\n";
                    return;
                }
                if (order.status == OrderStatus::Cancelled || order.status == OrderStatus::Refunded)
                {
                    cout << "该订单已经取消或退款，不能取餐。\n";
                    return;
                }
                if (order.pickupCode != code) 
                {
                    cout << "取餐码错误，验证失败。\n";
                    return;
                }

                order.status = OrderStatus::PickedUp;//更新订单状态为已取餐
                cout << "取餐成功，订单状态已更新为【已取餐】。\n";
                return;
            }
        }

        cout << "没有找到该订单。\n";
    }

    void payOrder() //支付订单函数
    {
        // 成员函数：当前学生对象支付自己的订单。
        if (orders.empty())
        {
            cout << "你目前没有订单。\n";
            return;
        }

        showOrders();
        int orderId = readInt("请输入要支付的订单编号：", 1, 999999);
        for (Order& order : orders)
        {
            if (order.orderId == orderId)
            {
                if (order.status == OrderStatus::Paid || order.status == OrderStatus::PickedUp)
                {
                    cout << "该订单已经支付，无需重复支付。\n";
                    return;
                }
                if (order.status == OrderStatus::Cancelled || order.status == OrderStatus::Refunded)
                {
                    cout << "该订单已经取消或退款，不能支付。\n";
                    return;
                }

                cout << "请选择支付方式：\n";
                cout << "1. 校园卡\n";
                cout << "2. 微信\n";
                cout << "3. 支付宝\n";
                int choice = readInt("请选择：", 1, 3);
                if (choice == 1) order.payMethod = "校园卡";
                if (choice == 2) order.payMethod = "微信";
                if (choice == 3) order.payMethod = "支付宝";

                order.status = OrderStatus::Paid;
                cout << "支付成功！金额：" << fixed << setprecision(2) << order.amount << " 元。\n";
                return;
            }
        }
        cout << "没有找到该订单。\n";
    }

    void cancelOrRefundOrder(vector<Dish>& dishes) //取消订单或退款函数
    {
        // 未支付订单取消，已支付订单退款，同时把库存补回去。
        if (orders.empty())
        {
            cout << "你目前没有订单。\n";
            return;
        }

        showOrders();
        int orderId = readInt("请输入要取消或退款的订单编号：", 1, 999999);
        for (Order& order : orders)
        {
            if (order.orderId == orderId)
            {
                if (order.status == OrderStatus::PickedUp)
                {
                    cout << "该订单已经取餐，不能取消或退款。\n";
                    return;
                }
                if (order.status == OrderStatus::Cancelled || order.status == OrderStatus::Refunded)
                {
                    cout << "该订单已经处理过，不能重复操作。\n";
                    return;
                }

                int confirm = readInt("确认取消/退款？1.确认  0.取消：", 0, 1);
                if (confirm == 0)
                {
                    cout << "已取消操作。\n";
                    return;
                }

                int index = findDishIndexById(dishes, order.dishId);
                if (index != -1)
                {
                    dishes[index].stock += order.quantity;
                }

                if (order.status == OrderStatus::Reserved)
                {
                    order.status = OrderStatus::Cancelled;
                    cout << "订单已取消，库存已恢复。\n";
                }
                else if (order.status == OrderStatus::Paid)
                {
                    order.status = OrderStatus::Refunded;
                    order.payMethod = "已退款";
                    cout << "订单已退款，退款金额：" << fixed << setprecision(2) << order.amount << " 元，库存已恢复。\n";
                }
                return;
            }
        }
        cout << "没有找到该订单。\n";
    }

    void modifySelf() //学生自己修改个人信息和密码
    {
        // 成员函数：当前学生对象只能修改自己的联系电话和密码，体现 this 对象管理自身数据。
        cout << "正在修改【" << name << "】的个人信息。直接回车表示保留原内容。\n";
        string newPhone = readText("联系电话（当前：" + phone + "）：", true);
        if (!newPhone.empty()) phone = newPhone;

        int changePwd = readInt("是否修改密码？1.是  0.否：", 0, 1);
        if (changePwd == 1)
        {
            string oldPwd = readText("请输入原密码：");
            if (oldPwd != password)
            {
                cout << "原密码错误，密码没有修改。\n";
            }
            else
            {
                string newPwd = readText("请输入新密码：");
                password = newPwd;
                cout << "密码修改成功。\n";
            }
        }
        cout << "个人信息修改完成。\n";
    }

    void rateDish(vector<Dish>& dishes) //学生评价菜品函数
    {
        // 学生可以对菜品进行 1-5 星评分并留言，评分会动态影响菜品平均分。
        if (dishes.empty())
        {
            cout << "当前没有菜品信息。\n";
            return;
        }

        showAllDishes(dishes);
        int dishId = readInt("请输入要评价的菜品编号：", 1, 999999);
        int index = findDishIndexById(dishes, dishId);
        if (index == -1)
        {
            cout << "没有找到该菜品。\n";
            return;
        }

        int score = readInt("请输入评分（1-5 星）：", 1, 5);
        string comment = readText("请输入评价留言：", true);

        Dish& dish = dishes[index];
        dish.rating = (dish.rating * dish.ratingCount + score) / (dish.ratingCount + 1);
        dish.ratingCount++;
        if (!comment.empty())
        {
            dish.comments.push_back(name + "：" + comment);
        }

        cout << "评价成功，菜品【" << dish.name << "】当前平均分为 "
             << fixed << setprecision(1) << dish.rating << " 星。\n";
    }

    void submitComplaint(vector<Complaint>& complaints, int& nextComplaintId) //学生提交投诉建议函数
    {
        // 投诉建议也是当前学生对象发起的动作，但保存到全局投诉列表中，方便管理员统一处理。
        cout << "请选择投诉建议类型：\n";
        cout << "1. 卫生问题\n";
        cout << "2. 服务问题\n";
        cout << "3. 菜品问题\n";
        cout << "4. 其他建议\n";
        int choice = readInt("请选择：", 1, 4);

        Complaint complaint;
        complaint.complaintId = nextComplaintId++;
        complaint.studentId = studentId;
        complaint.studentName = name;
        if (choice == 1) complaint.type = "卫生问题";
        if (choice == 2) complaint.type = "服务问题";
        if (choice == 3) complaint.type = "菜品问题";
        if (choice == 4) complaint.type = "其他建议";
        complaint.content = readText("请输入具体内容：");
        complaint.reply = "暂无回复";
        complaint.handled = false;

        complaints.push_back(complaint);
        cout << "提交成功，编号为：" << complaint.complaintId << "。管理员处理后可查看回复。\n";
    }

    void showOrders() const //打印当前学生对象的所有订单简略信息
    {
        // 成员函数：只显示当前学生自己的订单。
        if (orders.empty()) 
        {
            cout << "当前没有订单记录。\n";
            return;
        }

        // 说明：控制台中英文混排时，setw 对中文宽度的计算容易错位。
        // 为了让汇报演示更稳定，这里改成“订单卡片式”显示，不依赖表格对齐。
        printLine('-');
        cout << "我的订单记录如下：\n";
        for (const Order& order : orders) 
        {
            printLine('-');
            cout << "订单号：" << order.orderId << '\n';
            cout << "菜品：" << order.dishName << " × " << order.quantity << '\n';
            cout << "金额：" << fixed << setprecision(2) << order.amount << " 元\n";
            cout << "时段：" << order.mealTime << '\n';
            cout << "方式：" << order.mode << '\n';
            cout << "状态：" << orderStatusToString(order.status) << '\n';
            cout << "支付：" << order.payMethod << '\n';
            cout << "取餐码：" << order.pickupCode << '\n';
        }
        printLine('-');
    }
};

// ========================= 学生链表类 =========================
// StudentList 负责管理链表本身：头指针、插入、删除、查找、遍历。

class StudentList 
{
private:
    Student* head;

public:
    StudentList() : head(nullptr) {}//构造函数，初始化头指针为空

    ~StudentList() 
	{
        clear();//析构函数，释放链表内存
    }

    Student* getHead() const //返回头指针
	{
        return head;
    }

    bool isEmpty() const //判断链表是否为空
	{
        return head == nullptr;
    }

    void clear() //释放链表内存函数
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

    Student* findById(const string& studentId) const //根据学号查找学生对象函数，返回指针，如果找不到返回 nullptr
	{
        Student* current = head;
        while (current != nullptr) //遍历链表
		{
            if (current->getId() == studentId) //如果当前节点的学号与目标学号匹配，则返回该节点
			{
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }

    bool addStudent(Student* newStudent) //添加学生对象到链表函数，返回是否成功
	{
        if (newStudent == nullptr) //如果传入的学生对象为空指针，则添加失败
		{
            return false;
        }
        if (findById(newStudent->getId()) != nullptr) //如果链表中已经存在该学号的学生，则添加失败
		{
            return false;
        }

        // 头插法
        newStudent->next = head;
        head = newStudent;
        return true;//添加成功
    }

    bool removeById(const string& studentId) //根据学号删除学生对象函数，返回是否成功
	{
        Student* current = head;//当前节点指针，初始为头指针
        Student* previous = nullptr;//前一个节点指针，初始为 nullptr

        while (current != nullptr) 
        {
            if (current->getId() == studentId) //如果当前节点的学号与目标学号匹配，则删除该节点
            {
                if (previous == nullptr) //如果当前节点是头节点，则更新头指针
                {
                    head = current->next;
                } 
                 else                  //如果当前节点不是头节点，则更新前一个节点的 next 指针
                 {
                    previous->next = current->next;
                 }
                delete current;
                return true;
            }
            previous = current;//更新前一个节点指针为当前节点
            current = current->next;//更新当前节点指针为下一个节点
        }
        return false;
    }

    void showAllStudents() const //打印所有学生简略信息函数
    {
        if (isEmpty()) 
        {
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
             << setw(14) << "班级"
             << setw(14) << "联系电话" << '\n';
        printLine('-');

        Student* current = head;//从头指针开始遍历链表
        while (current != nullptr) 
        {
            current->showBrief();//打印当前学生的简略信息
            current = current->next;
        }
        printLine('-');
    }

    int countStudents() const //统计学生总人数函数
    {
        int count = 0;
        Student* current = head;
        while (current != nullptr) 
        {
            ++count;
            current = current->next;
        }
        return count;
    }
};

// ========================= 初始化数据 =========================

void initStudents(StudentList& students) //初始化学生数据函数
{//在堆上创建学生对象，返回地址给链表，链表负责管理这些对象的生命周期
    students.addStudent(new Student("20260001", "111111", "李明", "男", "2007-03-12", "大一", "软件工程", "13800000001", "软件一班"));
    students.addStudent(new Student("20260002", "222222", "王雨", "女", "2007-07-05", "大一", "计算机科学", "13800000002", "计科一班"));
    students.addStudent(new Student("20260003", "333333", "张辰", "男", "2006-11-20", "大二", "网络工程", "13800000003", "网工一班"));
}

void initDishes(vector<Dish>& dishes) //初始化菜品数据函数
{//在栈上创建菜品对象，直接 push_back 到 vector 中，vector 负责管理这些对象的生命周期
    //结构体初始化列表语法，按顺序给结构体成员赋值
    dishes.push_back({1001, "鸡蛋灌饼", "早餐", 6.50, 30, "咸香", "蛋白质、碳水", "鸡蛋、面粉", 4.6, 1, vector<string>()});
    dishes.push_back({1002, "小米粥", "早餐", 3.00, 40, "清淡", "碳水、膳食纤维", "无明显过敏源", 4.4, 1, vector<string>()});
    dishes.push_back({1003, "红烧鸡腿", "午餐", 12.00, 25, "咸鲜", "蛋白质、脂肪", "大豆", 4.7, 1, vector<string>()});
    dishes.push_back({1004, "番茄炒蛋", "午餐", 8.00, 20, "酸甜", "维生素、蛋白质", "鸡蛋", 4.5, 1, vector<string>()});
    dishes.push_back({1005, "麻辣香锅", "晚餐", 18.00, 15, "麻辣", "蛋白质、脂肪", "花生、芝麻", 4.8, 1, vector<string>()});
    dishes.push_back({1006, "素炒西兰花", "特色菜", 7.00, 18, "清爽", "维生素、膳食纤维", "无明显过敏源", 4.3, 1, vector<string>()});
}

void initClasses(vector<ClassInfo>& classes) //初始化班级数据函数
{
    classes.push_back({"软件一班", "大一", "软件工程", "刘老师", "软件工程专业基础班"});
    classes.push_back({"计科一班", "大一", "计算机科学", "王老师", "计算机科学专业基础班"});
    classes.push_back({"网工一班", "大二", "网络工程", "赵老师", "网络工程专业班"});
}

// ========================= 管理员全局函数：学生管理 =========================
// 这些函数没有 this 指针，不属于某一个学生对象。
Student* createStudentFromInput() //从终端读取学生信息，然后 new 出一个 Student 对象，并返回这个对象的指针
{
    string id = readText("请输入学号：");
    string password = readText("请输入初始密码：");
    string name = readText("请输入姓名：");
    string gender = readText("请输入性别：");
    string birthDate = readText("请输入出生日期：");
    string grade = readText("请输入年级：");
    string major = readText("请输入专业：");
    string className = readText("请输入班级：");
    string phone = readText("请输入联系电话：");
    return new Student(id, password, name, gender, birthDate, grade, major, phone, className);//在堆上创建学生对象，返回指针
}

void adminAddStudent(StudentList& students) //管理员添加学生函数，参数是学生链表的引用
{
    Student* newStudent = createStudentFromInput();//新对象的指针，传给下一行
    if (students.addStudent(newStudent)) //先调用 addStudent() 尝试添加，如果成功则返回 true，否则返回 false
    {
        cout << "学生添加成功。\n";
    } else {
        cout << "添加失败：学号可能已经存在。\n";
        delete newStudent;
    }
}

void adminDeleteStudent(StudentList& students) 
{
    string id = readText("请输入要删除的学生学号：");
    Student* student = students.findById(id);//通过学号得到学生对象指针
    if (student == nullptr) {
        cout << "没有找到该学生。\n";
        return;
    }

    cout << "即将删除学生：" << student->getName() << "（" << student->getId() << "）\n";
    int confirm = readInt("确认删除？1.确认  0.取消：", 0, 1);
    if (confirm == 1 && students.removeById(id)) //调用 removeById() 尝试删除，如果成功则返回 true，否则返回 false
    {
        cout << "删除成功。\n";
    } else {
        cout << "已取消删除。\n";
    }
}

void adminSearchStudent(const StudentList& students) //管理员查询学生及其订单函数，参数是学生链表的常量引用
{
    string id = readText("请输入要查询的学生学号：");
    Student* student = students.findById(id);//通过学号得到学生对象指针
    if (student == nullptr) 
    {
        cout << "没有找到该学生。\n";
        return;
    }

    student->showProfile();//打印学生详细信息
    cout << "该学生订单：\n";
    student->showOrders();//打印学生订单简略信息
}

void adminModifyStudent(StudentList& students) //管理员修改学生信息函数
{
    string id = readText("请输入要修改的学生学号：");
    Student* student = students.findById(id);//通过学号得到学生对象指针
    if (student == nullptr) 
    {
        cout << "没有找到该学生。\n";
        return;
    }

    student->modifyByAdmin();//调用学生对象的成员函数修改信息
}

void studentManageMenu(StudentList& students) //管理员学生数据管理菜单函数
{
    while (true) 
    {
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

        switch (choice) 
        {
            case 1:
                students.showAllStudents();//打印所有学生简略信息
                cout << "学生总人数：" << students.countStudents() << '\n';
                break;
            case 2:
                adminAddStudent(students);//管理员添加学生
                break;
            case 3:
                adminDeleteStudent(students);//管理员删除学生
                break;
            case 4:
                adminModifyStudent(students);//管理员修改学生信息
                break;
            case 5:
                adminSearchStudent(students);//管理员查询学生及其订单
                break;
        }
        pauseScreen();//暂停屏幕，等待用户按任意键继续
    }
}

// ========================= 管理员全局函数：菜品管理 =========================

void adminAddDish(vector<Dish>& dishes) //管理员添加菜品函数，参数是菜品向量的引用
{
    Dish dish;//临时菜品对象，用于存储用户输入的菜品信息  
    dish.id = getNextDishId(dishes);//调用函数生成新菜品编号
    cout << "新菜品编号自动生成：" << dish.id << '\n';
    dish.name = readText("请输入菜品名称：");
    dish.category = readText("请输入菜品分类：");
    dish.price = readDouble("请输入价格：", 0.1, 9999.0);
    dish.stock = readInt("请输入库存数量：", 0, 99999);
    dish.flavor = readText("请输入口味：");
    dish.nutrition = readText("请输入营养成分：");
    dish.allergen = readText("请输入过敏源信息：");
    dish.rating = readDouble("请输入评分（1-5）：", 1.0, 5.0);
    dish.ratingCount = 1;
    dish.comments.clear();

    dishes.push_back(dish);//将临时菜品对象添加到菜品向量中，vector 会自动管理内存
    cout << "菜品添加成功。\n";
}

void adminDeleteDish(vector<Dish>& dishes) //管理员删除菜品函数
{
    showAllDishes(dishes);//打印所有菜品简略信息，方便管理员选择要删除的菜品
    int dishId = readInt("请输入要删除的菜品编号：", 1, 999999);
    int index = findDishIndexById(dishes, dishId);//根据菜品编号查找菜品，返回 vector 下标；找不到返回 -1。
    if (index == -1) 
    {
        cout << "没有找到该菜品。\n";
        return;
    }

    cout << "即将删除菜品：" << dishes[index].name << '\n';
    int confirm = readInt("确认删除？1.确认  0.取消：", 0, 1);
    if (confirm == 1) {
        dishes.erase(dishes.begin() + index);//调用 vector 的 erase() 方法删除指定下标的元素
        cout << "删除成功。\n";
    } else {
        cout << "已取消删除。\n";
    }
}

void adminModifyDish(vector<Dish>& dishes) //管理员修改菜品函数
{
    showAllDishes(dishes);//打印所有菜品简略信息，方便管理员选择要修改的菜品
    int dishId = readInt("请输入要修改的菜品编号：", 1, 999999);
    int index = findDishIndexById(dishes, dishId);//根据菜品编号查找菜品，返回 vector 下标；找不到返回 -1。
    if (index == -1) 
    {
        cout << "没有找到该菜品。\n";
        return;
    }

    Dish& dish = dishes[index];//临时菜品对象的引用，方便后续修改
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

void dishManageMenu(vector<Dish>& dishes) //管理员菜品管理菜单函数
{
    while (true) 
    {
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

        switch (choice) 
        {
            case 1:
                showAllDishes(dishes);//打印所有菜品简略信息
                break;
            case 2:
                adminAddDish(dishes);//管理员添加菜品
                break;
            case 3:
                adminDeleteDish(dishes);//管理员删除菜品
                break;
            case 4:
                adminModifyDish(dishes);//管理员修改菜品
                break;
        }
        pauseScreen();//暂停屏幕，等待用户按任意键继续
    }
}

// ========================= 管理员全局函数：订单查询 =========================

void adminShowAllOrders(const StudentList& students) //打印所有学生的所有订单函数
{
    Student* current = students.getHead();//从头指针开始遍历链表
    bool hasOrder = false;//标记是否有订单记录

    // 说明：管理员查看全部订单时也使用卡片式显示，避免中文表格在不同终端中错位。
    printLine('-');
    cout << "全部订单记录如下：\n";

    while (current != nullptr) //遍历学生链表
	{
        const vector<Order>& orders = current->getOrders();//调用当前学生的成员函数获取订单向量的常量引用作为临时变量
        for (const Order& order : orders) //遍历当前学生的订单向量
		{
            hasOrder = true;//标记有订单记录
            printLine('-');
            cout << "学生：" << current->getName() << "（" << current->getId() << "）\n";
            cout << "订单号：" << order.orderId << '\n';
            cout << "菜品：" << order.dishName << " × " << order.quantity << '\n';
            cout << "金额：" << fixed << setprecision(2) << order.amount << " 元\n";
            cout << "状态：" << orderStatusToString(order.status) << '\n';
            cout << "支付：" << order.payMethod << '\n';
            cout << "取餐码：" << order.pickupCode << '\n';
        }
        current = current->next;
    }

    if (!hasOrder)//只有当所有学生都没有订单时，才会输出提示信息
	{
        cout << "当前还没有任何订单。\n";
    }
    printLine('-');
}

void adminSearchOrdersByDish(const StudentList& students) //按菜品名称查询订单函数
{
    string keyword = readText("请输入要查询的菜品名称关键字：");
    Student* current = students.getHead();
    bool found = false;//标记是否找到相关订单

    printLine('-');
    cout << "订购包含【" << keyword << "】的菜品的学生：\n";
    printLine('-');

    while (current != nullptr) //遍历学生链表
    {
        const vector<Order>& orders = current->getOrders();//调用当前学生的成员函数获取订单向量的常量引用作为临时变量
        for (const Order& order : orders) 
        {
            if (order.dishName.find(keyword) != string::npos) //如果菜品名称中包含关键字，则输出该订单信息
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

    if (!found) 
    {
        cout << "没有查询到相关订单。\n";
    }
}

void adminSearchOrdersByStudent(const StudentList& students) //按学生学号查询订单函数
{
    string id = readText("请输入学生学号：");
    Student* student = students.findById(id);
    if (student == nullptr) {
        cout << "没有找到该学生。\n";
        return;
    }

    cout << "学生【" << student->getName() << "】的订单信息：\n";
    student->showOrders();
}

void orderQueryMenu(const StudentList& students) //管理员订单查询菜单函数
{
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

// ========================= 管理员全局函数：数据统计分析 =========================

void adminDataAnalysis(const StudentList& students) //管理员数据统计分析函数
{
    int totalStudents = students.countStudents();
    int totalOrders = 0;
    int reservedCount = 0, paidCount = 0, pickedCount = 0, cancelledCount = 0, refundedCount = 0;
    int breakfastCount = 0, lunchCount = 0, dinnerCount = 0;
    int dineInCount = 0, takeAwayCount = 0, deliveryCount = 0;
    int cardPayCount = 0, wechatPayCount = 0, alipayCount = 0;
    double totalRevenue = 0.0;

    vector<int> dishIds;
    vector<string> dishNames;
    vector<int> dishQuantities;

    Student* current = students.getHead();
    while (current != nullptr)
    {
        const vector<Order>& orders = current->getOrders();
        for (const Order& order : orders)
        {
            totalOrders++;
            if (order.status == OrderStatus::Reserved) reservedCount++;
            if (order.status == OrderStatus::Paid) paidCount++;
            if (order.status == OrderStatus::PickedUp) pickedCount++;
            if (order.status == OrderStatus::Cancelled) cancelledCount++;
            if (order.status == OrderStatus::Refunded) refundedCount++;

            if (shouldCountRevenue(order.status))
            {
                totalRevenue += order.amount;
            }

            if (order.mealTime.find("早餐") != string::npos) breakfastCount++;
            if (order.mealTime.find("午餐") != string::npos) lunchCount++;
            if (order.mealTime.find("晚餐") != string::npos) dinnerCount++;

            if (order.mode == "堂食") dineInCount++;
            if (order.mode == "自提") takeAwayCount++;
            if (order.mode == "外卖") deliveryCount++;

            if (order.payMethod == "校园卡") cardPayCount++;
            if (order.payMethod == "微信") wechatPayCount++;
            if (order.payMethod == "支付宝") alipayCount++;

            bool foundDish = false;
            for (size_t i = 0; i < dishIds.size(); ++i)
            {
                if (dishIds[i] == order.dishId)
                {
                    dishQuantities[i] += order.quantity;
                    foundDish = true;
                    break;
                }
            }
            if (!foundDish)
            {
                dishIds.push_back(order.dishId);
                dishNames.push_back(order.dishName);
                dishQuantities.push_back(order.quantity);
            }
        }
        current = current->next;
    }

    int bestIndex = -1;
    for (size_t i = 0; i < dishQuantities.size(); ++i)
    {
        if (bestIndex == -1 || dishQuantities[i] > dishQuantities[bestIndex])
        {
            bestIndex = static_cast<int>(i);
        }
    }

    printLine('-');
    cout << "学生总人数：" << totalStudents << '\n';
    cout << "订单总数：" << totalOrders << '\n';
    cout << "待支付：" << reservedCount << "，已支付：" << paidCount
         << "，已取餐：" << pickedCount << "，已取消：" << cancelledCount
         << "，已退款：" << refundedCount << '\n';
    cout << "有效营收：" << fixed << setprecision(2) << totalRevenue << " 元\n";
    if (bestIndex != -1)
    {
        cout << "热销菜品：" << dishNames[bestIndex] << "，累计销售 " << dishQuantities[bestIndex] << " 份\n";
    }
    else
    {
        cout << "热销菜品：暂无订单数据\n";
    }
    cout << "用餐时段统计：早餐 " << breakfastCount << "，午餐 " << lunchCount << "，晚餐 " << dinnerCount << '\n';
    cout << "用餐方式统计：堂食 " << dineInCount << "，自提 " << takeAwayCount << "，外卖 " << deliveryCount << '\n';
    cout << "支付方式统计：校园卡 " << cardPayCount << "，微信 " << wechatPayCount << "，支付宝 " << alipayCount << '\n';
    printLine('-');
}

void dataAnalysisMenu(const StudentList& students) //管理员数据统计菜单函数
{
    while (true)
    {
        printLine();
        cout << "管理员菜单 > 数据统计分析\n";
        printLine();
        cout << "1. 查看综合统计报表\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 1);
        if (choice == 0) return;
        if (choice == 1) adminDataAnalysis(students);
        pauseScreen();
    }
}

// ========================= 管理员全局函数：菜品评价查看 =========================

void adminShowDishComments(const vector<Dish>& dishes) //管理员查看菜品评价函数
{
    if (dishes.empty())
    {
        cout << "当前没有菜品信息。\n";
        return;
    }

    showAllDishes(dishes);
    int dishId = readInt("请输入要查看评价的菜品编号：", 1, 999999);
    int index = findDishIndexById(dishes, dishId);
    if (index == -1)
    {
        cout << "没有找到该菜品。\n";
        return;
    }

    showDishDetail(dishes[index]);
}

// ========================= 管理员全局函数：投诉建议处理 =========================

void showAllComplaints(const vector<Complaint>& complaints) //打印所有投诉建议函数
{
    if (complaints.empty())
    {
        cout << "当前没有投诉建议。\n";
        return;
    }

    printLine('-');
    cout << left
         << setw(8) << "编号"
         << setw(12) << "学生"
         << setw(14) << "类型"
         << setw(10) << "状态"
         << "内容" << '\n';
    printLine('-');

    for (const Complaint& complaint : complaints)
    {
        cout << left
             << setw(8) << complaint.complaintId
             << setw(12) << complaint.studentName
             << setw(14) << complaint.type
             << setw(10) << (complaint.handled ? "已处理" : "未处理")
             << complaint.content << '\n';
        cout << "管理员回复：" << complaint.reply << '\n';
    }
    printLine('-');
}

void adminReplyComplaint(vector<Complaint>& complaints) //管理员回复投诉建议函数
{
    showAllComplaints(complaints);
    if (complaints.empty()) return;

    int id = readInt("请输入要回复的投诉建议编号：", 1, 999999);
    for (Complaint& complaint : complaints)
    {
        if (complaint.complaintId == id)
        {
            complaint.reply = readText("请输入管理员回复：");
            complaint.handled = true;
            cout << "回复完成。\n";
            return;
        }
    }
    cout << "没有找到该编号。\n";
}

void complaintManageMenu(vector<Complaint>& complaints) //管理员投诉建议处理菜单函数
{
    while (true)
    {
        printLine();
        cout << "管理员菜单 > 投诉建议处理\n";
        printLine();
        cout << "1. 查看全部投诉建议\n";
        cout << "2. 回复投诉建议\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 2);
        if (choice == 0) return;

        switch (choice)
        {
            case 1:
                showAllComplaints(complaints);
                break;
            case 2:
                adminReplyComplaint(complaints);
                break;
        }
        pauseScreen();
    }
}

// ========================= 管理员全局函数：班级信息管理 =========================

void showAllClasses(const vector<ClassInfo>& classes) //打印所有班级信息函数
{
    if (classes.empty())
    {
        cout << "当前没有班级信息。\n";
        return;
    }

    printLine('-');
    cout << left
         << setw(14) << "班级"
         << setw(10) << "年级"
         << setw(18) << "专业"
         << setw(12) << "负责人"
         << "备注" << '\n';
    printLine('-');

    for (const ClassInfo& classInfo : classes)
    {
        cout << left
             << setw(14) << classInfo.className
             << setw(10) << classInfo.grade
             << setw(18) << classInfo.major
             << setw(12) << classInfo.advisor
             << classInfo.remark << '\n';
    }
    printLine('-');
}

int findClassIndexByName(const vector<ClassInfo>& classes, const string& className) //按班级名称查找班级
{
    for (size_t i = 0; i < classes.size(); ++i)
    {
        if (classes[i].className == className)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void adminAddClass(vector<ClassInfo>& classes) //管理员添加班级函数
{
    ClassInfo classInfo;
    classInfo.className = readText("请输入班级名称：");
    if (findClassIndexByName(classes, classInfo.className) != -1)
    {
        cout << "该班级已经存在。\n";
        return;
    }
    classInfo.grade = readText("请输入年级：");
    classInfo.major = readText("请输入专业：");
    classInfo.advisor = readText("请输入负责人：");
    classInfo.remark = readText("请输入备注：", true);
    classes.push_back(classInfo);
    cout << "班级添加成功。\n";
}

void adminModifyClass(vector<ClassInfo>& classes) //管理员修改班级函数
{
    showAllClasses(classes);
    string className = readText("请输入要修改的班级名称：");
    int index = findClassIndexByName(classes, className);
    if (index == -1)
    {
        cout << "没有找到该班级。\n";
        return;
    }

    ClassInfo& classInfo = classes[index];
    cout << "正在修改班级【" << classInfo.className << "】。直接回车表示保留原内容。\n";
    string newGrade = readText("年级（当前：" + classInfo.grade + "）：", true);
    string newMajor = readText("专业（当前：" + classInfo.major + "）：", true);
    string newAdvisor = readText("负责人（当前：" + classInfo.advisor + "）：", true);
    string newRemark = readText("备注（当前：" + classInfo.remark + "）：", true);

    if (!newGrade.empty()) classInfo.grade = newGrade;
    if (!newMajor.empty()) classInfo.major = newMajor;
    if (!newAdvisor.empty()) classInfo.advisor = newAdvisor;
    if (!newRemark.empty()) classInfo.remark = newRemark;

    cout << "班级信息修改完成。\n";
}

void adminCountOrdersByClass(const StudentList& students) //按班级统计订餐人数和订单数
{
    string className = readText("请输入班级名称：");
    Student* current = students.getHead();
    int studentCount = 0;
    int orderStudentCount = 0;
    int orderCount = 0;
    double amount = 0.0;

    while (current != nullptr)
    {
        if (current->getClassName() == className)
        {
            studentCount++;
            const vector<Order>& orders = current->getOrders();
            if (!orders.empty()) orderStudentCount++;
            for (const Order& order : orders)
            {
                orderCount++;
                if (shouldCountRevenue(order.status))
                {
                    amount += order.amount;
                }
            }
        }
        current = current->next;
    }

    printLine('-');
    cout << "班级：" << className << '\n';
    cout << "班级学生人数：" << studentCount << '\n';
    cout << "有订餐记录的人数：" << orderStudentCount << '\n';
    cout << "订单数量：" << orderCount << '\n';
    cout << "有效消费金额：" << fixed << setprecision(2) << amount << " 元\n";
    printLine('-');
}

void adminSearchOrdersByClass(const StudentList& students) //按班级查阅订餐信息
{
    string className = readText("请输入班级名称：");
    Student* current = students.getHead();
    bool found = false;

    printLine('-');
    cout << "班级【" << className << "】订餐信息：\n";
    printLine('-');

    while (current != nullptr)
    {
        if (current->getClassName() == className)
        {
            const vector<Order>& orders = current->getOrders();
            for (const Order& order : orders)
            {
                found = true;
                cout << "学生：" << current->getName()
                     << "，学号：" << current->getId()
                     << "，菜品：" << order.dishName
                     << "，数量：" << order.quantity
                     << "，金额：" << fixed << setprecision(2) << order.amount
                     << "，状态：" << orderStatusToString(order.status) << '\n';
            }
        }
        current = current->next;
    }

    if (!found)
    {
        cout << "该班级暂无订餐记录。\n";
    }
}

void classManageMenu(vector<ClassInfo>& classes, const StudentList& students) //管理员班级管理菜单函数
{
    while (true)
    {
        printLine();
        cout << "管理员菜单 > 班级信息管理\n";
        printLine();
        cout << "1. 查看全部班级\n";
        cout << "2. 增加班级\n";
        cout << "3. 修改班级信息\n";
        cout << "4. 统计班级订餐情况\n";
        cout << "5. 按班级查阅订餐信息\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 5);
        if (choice == 0) return;

        switch (choice)
        {
            case 1:
                showAllClasses(classes);
                break;
            case 2:
                adminAddClass(classes);
                break;
            case 3:
                adminModifyClass(classes);
                break;
            case 4:
                adminCountOrdersByClass(students);
                break;
            case 5:
                adminSearchOrdersByClass(students);
                break;
        }
        pauseScreen();
    }
}

// 因为主菜单中需要调用自动保存函数，而文件读写函数写在后面，所以这里先声明。
void saveAllData(const StudentList& students, const vector<Dish>& dishes, const vector<ClassInfo>& classes, const vector<Complaint>& complaints, int nextOrderId, int nextComplaintId);

// ========================= 登录与主菜单 =========================

bool adminLogin() //管理员登录函数
{
    string name = readText("管理员账号：");
    string password = readText("管理员密码：");

    if (name == ADMIN_NAME && password == ADMIN_PASSWORD) //如果账号和密码匹配，则登录成功
    {
        cout << "管理员登录成功。\n";
        return true;
    }
    cout << "账号或密码错误。\n";
    return false;
}

Student* studentLogin(StudentList& students) //学生登录函数
{
    string id = readText("请输入学号：");
    string password = readText("请输入密码：");

    Student* student = students.findById(id);//通过学号查找学生对象指针
    if (student == nullptr || !student->checkPassword(password)) //如果找不到学生对象或密码不匹配，则登录失败
    {
        cout << "学号或密码错误。\n";
        return nullptr;
    }

    cout << "学生登录成功，欢迎你，" << student->getName() << "！\n";
    return student;
}

void registerStudent(StudentList& students) //学生注册函数
{
    cout << "\n【学生注册】\n";
    Student* newStudent = createStudentFromInput();//从终端读取学生信息，然后 new 出一个 Student 对象，并返回这个对象的指针
    if (students.addStudent(newStudent)) //尝试添加学生对象到链表，如果成功则返回 true，否则返回 false
    {
        cout << "注册成功，请返回主菜单登录。\n";
    } else {
        cout << "注册失败：该学号已经存在。\n";
        delete newStudent;
    }
}

void adminMenu(StudentList& students, vector<Dish>& dishes, vector<ClassInfo>& classes, vector<Complaint>& complaints) //管理员菜单函数
{
    while (true) 
    {
        printLine();
        cout << "校园食堂信息管理系统 > 管理员菜单\n";
        printLine();
        cout << "1. 学生数据管理\n";
        cout << "2. 菜品管理\n";
        cout << "3. 订单查询\n";
        cout << "4. 数据统计分析\n";
        cout << "5. 班级信息管理\n";
        cout << "6. 查看菜品评价\n";
        cout << "7. 投诉建议处理\n";
        cout << "0. 退出管理员菜单\n";

        int choice = readInt("请选择：", 0, 7);
        if (choice == 0) return;

        switch (choice) {
            case 1:
                studentManageMenu(students);//管理员学生数据管理菜单
                break;
            case 2:
                dishManageMenu(dishes);//管理员菜品管理菜单
                break;
            case 3:
                orderQueryMenu(students);//管理员订单查询菜单
                break;
            case 4:
                dataAnalysisMenu(students);//管理员数据统计分析菜单
                break;
            case 5:
                classManageMenu(classes, students);//管理员班级信息管理菜单
                break;
            case 6:
                adminShowDishComments(dishes);//管理员查看菜品评价
                pauseScreen();
                break;
            case 7:
                complaintManageMenu(complaints);//管理员投诉建议处理菜单
                break;
        }
    }
}

void studentMenu(Student* student, vector<Dish>& dishes, int& nextOrderId, vector<Complaint>& complaints, int& nextComplaintId) //学生菜单函数，参数是学生对象指针、菜品向量引用和订单编号引用
{
    // student 是指针，下面通过 student->成员函数 调用。
  
    while (true) 
    {
        printLine();
        cout << "校园食堂信息管理系统 > 学生菜单\n";
        printLine();
        cout << "当前用户：" << student->getName() << "（" << student->getId() << "）\n";
        cout << "1. 查看菜品\n";
        cout << "2. 在线订餐\n";
        cout << "3. 支付订单\n";
        cout << "4. 取餐验证\n";
        cout << "5. 查询我的订单\n";
        cout << "6. 查看个人信息\n";
        cout << "7. 修改个人信息/密码\n";
        cout << "8. 评价菜品\n";
        cout << "9. 提交投诉建议\n";
        cout << "10. 取消订单/退款\n";
        cout << "0. 退出学生菜单\n";

        int choice = readInt("请选择：", 0, 10);
        if (choice == 0) return;

        switch (choice) {
            case 1:
                student->viewDishes(dishes);
                break;
            case 2:
                student->placeOrder(dishes, nextOrderId);
                break;
            case 3:
                student->payOrder();
                break;
            case 4:
                student->pickupOrder();
                break;
            case 5:
                student->showOrders();
                break;
            case 6:
                student->showProfile();
                break;
            case 7:
                student->modifySelf();
                break;
            case 8:
                student->rateDish(dishes);
                break;
            case 9:
                student->submitComplaint(complaints, nextComplaintId);
                break;
            case 10:
                student->cancelOrRefundOrder(dishes);
                break;
        }
        pauseScreen();
    }
}

void mainMenu(StudentList& students, vector<Dish>& dishes, int& nextOrderId, vector<ClassInfo>& classes, vector<Complaint>& complaints, int& nextComplaintId) //主菜单函数，参数是学生链表引用、菜品向量引用和订单编号引用
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
        if (choice == 0) 
        {
            cout << "感谢使用校园食堂信息管理系统，再见！\n";
            return;
        }

        switch (choice) 
        {
            case 1:
                if (adminLogin()) //如果管理员登录成功，则进入管理员菜单
                {
                    adminMenu(students, dishes, classes, complaints);//管理员菜单函数，参数是学生链表引用和菜品向量引用
                }
                pauseScreen();
                break;
            case 2: 
            {
                Student* student = studentLogin(students);//如果学生登录成功，则进入学生菜单
                if (student != nullptr) 
                {
                    studentMenu(student, dishes, nextOrderId, complaints, nextComplaintId);//学生菜单函数，参数是学生对象指针、菜品向量引用和订单编号引用
                }
                pauseScreen();
                break;
            }
            case 3:
                registerStudent(students);//学生注册函数，参数是学生链表引用
                pauseScreen();
                break;
        }

        // 每次从管理员菜单或学生菜单返回主菜单时，自动保存一次数据。
        // 这样即使还没有完全退出系统，也能在程序运行目录下看到 txt 数据文件。
        saveAllData(students, dishes, classes, complaints, nextOrderId, nextComplaintId);
        cout << "\n数据已自动保存到程序运行目录下。\n";
    }
}

// ========================= 文件读写函数 =========================
// 这里使用简单的文本文件保存数据，便于大一阶段理解和调试。

void saveStudentsToFile(const StudentList& students) //保存学生基本信息
{
    ofstream fout(STUDENT_FILE.c_str());
    Student* current = students.getHead();
    while (current != nullptr)
    {
        fout << current->getId() << '|'
             << current->getPasswordForFile() << '|'
             << current->getName() << '|'
             << current->getGender() << '|'
             << current->getBirthDate() << '|'
             << current->getGrade() << '|'
             << current->getMajor() << '|'
             << current->getClassName() << '|'
             << current->getPhone() << '\n';
        current = current->next;
    }
}

void saveDishesToFile(const vector<Dish>& dishes) //保存菜品信息和评价留言
{
    ofstream fout(DISH_FILE.c_str());
    for (const Dish& dish : dishes)
    {
        fout << dish.id << '|'
             << dish.name << '|'
             << dish.category << '|'
             << dish.price << '|'
             << dish.stock << '|'
             << dish.flavor << '|'
             << dish.nutrition << '|'
             << dish.allergen << '|'
             << dish.rating << '|'
             << dish.ratingCount << '|'
             << joinStrings(dish.comments, "##") << '\n';
    }
}

void saveOrdersToFile(const StudentList& students) //保存所有学生订单
{
    ofstream fout(ORDER_FILE.c_str());
    Student* current = students.getHead();
    while (current != nullptr)
    {
        const vector<Order>& orders = current->getOrders();
        for (const Order& order : orders)
        {
            fout << current->getId() << '|'
                 << order.orderId << '|'
                 << order.dishId << '|'
                 << order.dishName << '|'
                 << order.quantity << '|'
                 << order.amount << '|'
                 << order.mealTime << '|'
                 << order.mode << '|'
                 << orderStatusToInt(order.status) << '|'
                 << order.pickupCode << '|'
                 << order.payMethod << '\n';
        }
        current = current->next;
    }
}

void saveClassesToFile(const vector<ClassInfo>& classes) //保存班级信息
{
    ofstream fout(CLASS_FILE.c_str());
    for (const ClassInfo& classInfo : classes)
    {
        fout << classInfo.className << '|'
             << classInfo.grade << '|'
             << classInfo.major << '|'
             << classInfo.advisor << '|'
             << classInfo.remark << '\n';
    }
}

void saveComplaintsToFile(const vector<Complaint>& complaints) //保存投诉建议
{
    ofstream fout(COMPLAINT_FILE.c_str());
    for (const Complaint& complaint : complaints)
    {
        fout << complaint.complaintId << '|'
             << complaint.studentId << '|'
             << complaint.studentName << '|'
             << complaint.type << '|'
             << complaint.content << '|'
             << complaint.reply << '|'
             << complaint.handled << '\n';
    }
}

void saveCountersToFile(int nextOrderId, int nextComplaintId) //保存下一个订单编号和投诉编号
{
    ofstream fout(COUNTER_FILE.c_str());
    fout << nextOrderId << '|' << nextComplaintId << '\n';
}

void saveAllData(const StudentList& students, const vector<Dish>& dishes, const vector<ClassInfo>& classes, const vector<Complaint>& complaints, int nextOrderId, int nextComplaintId)
{
    saveStudentsToFile(students);
    saveDishesToFile(dishes);
    saveOrdersToFile(students);
    saveClassesToFile(classes);
    saveComplaintsToFile(complaints);
    saveCountersToFile(nextOrderId, nextComplaintId);
    cout << "系统数据已自动保存。\n";
}

bool loadStudentsFromFile(StudentList& students) //读取学生基本信息
{
    ifstream fin(STUDENT_FILE.c_str());
    if (!fin) return false;

    students.clear();
    string line;
    while (getline(fin, line))
    {
        if (line.empty()) continue;
        vector<string> data = splitLine(line, '|');
        if (data.size() >= 9)
        {
            students.addStudent(new Student(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[8], data[7]));
        }
    }
    return !students.isEmpty();
}

bool loadDishesFromFile(vector<Dish>& dishes) //读取菜品信息
{
    ifstream fin(DISH_FILE.c_str());
    if (!fin) return false;

    dishes.clear();
    string line;
    while (getline(fin, line))
    {
        if (line.empty()) continue;
        vector<string> data = splitLine(line, '|');
        if (data.size() >= 10)
        {
            Dish dish;
            dish.id = atoi(data[0].c_str());
            dish.name = data[1];
            dish.category = data[2];
            dish.price = atof(data[3].c_str());
            dish.stock = atoi(data[4].c_str());
            dish.flavor = data[5];
            dish.nutrition = data[6];
            dish.allergen = data[7];
            dish.rating = atof(data[8].c_str());
            dish.ratingCount = atoi(data[9].c_str());
            if (dish.ratingCount <= 0) dish.ratingCount = 1;
            if (data.size() >= 11 && !data[10].empty())
            {
                dish.comments = splitLine(data[10], '#');
                // 上面用单个 # 拆分会把 ## 看作两个分隔符，可能产生空串，下面清理空串。
                vector<string> cleaned;
                for (const string& comment : dish.comments)
                {
                    if (!comment.empty()) cleaned.push_back(comment);
                }
                dish.comments = cleaned;
            }
            dishes.push_back(dish);
        }
    }
    return !dishes.empty();
}

void loadOrdersFromFile(StudentList& students, int& nextOrderId) //读取订单信息
{
    ifstream fin(ORDER_FILE.c_str());
    if (!fin) return;

    string line;
    while (getline(fin, line))
    {
        if (line.empty()) continue;
        vector<string> data = splitLine(line, '|');
        if (data.size() >= 11)
        {
            Student* student = students.findById(data[0]);
            if (student != nullptr)
            {
                Order order;
                order.orderId = atoi(data[1].c_str());
                order.dishId = atoi(data[2].c_str());
                order.dishName = data[3];
                order.quantity = atoi(data[4].c_str());
                order.amount = atof(data[5].c_str());
                order.mealTime = data[6];
                order.mode = data[7];
                order.status = intToOrderStatus(atoi(data[8].c_str()));
                order.pickupCode = data[9];
                order.payMethod = data[10];
                student->addOrderFromFile(order);
                if (order.orderId >= nextOrderId)
                {
                    nextOrderId = order.orderId + 1;
                }
            }
        }
    }
}

bool loadClassesFromFile(vector<ClassInfo>& classes) //读取班级信息
{
    ifstream fin(CLASS_FILE.c_str());
    if (!fin) return false;

    classes.clear();
    string line;
    while (getline(fin, line))
    {
        if (line.empty()) continue;
        vector<string> data = splitLine(line, '|');
        if (data.size() >= 5)
        {
            classes.push_back({data[0], data[1], data[2], data[3], data[4]});
        }
    }
    return !classes.empty();
}

void loadComplaintsFromFile(vector<Complaint>& complaints, int& nextComplaintId) //读取投诉建议
{
    ifstream fin(COMPLAINT_FILE.c_str());
    if (!fin) return;

    complaints.clear();
    string line;
    while (getline(fin, line))
    {
        if (line.empty()) continue;
        vector<string> data = splitLine(line, '|');
        if (data.size() >= 7)
        {
            Complaint complaint;
            complaint.complaintId = atoi(data[0].c_str());
            complaint.studentId = data[1];
            complaint.studentName = data[2];
            complaint.type = data[3];
            complaint.content = data[4];
            complaint.reply = data[5];
            complaint.handled = atoi(data[6].c_str()) != 0;
            complaints.push_back(complaint);
            if (complaint.complaintId >= nextComplaintId)
            {
                nextComplaintId = complaint.complaintId + 1;
            }
        }
    }
}

void loadCountersFromFile(int& nextOrderId, int& nextComplaintId) //读取编号计数器
{
    ifstream fin(COUNTER_FILE.c_str());
    if (!fin) return;

    string line;
    if (getline(fin, line))
    {
        vector<string> data = splitLine(line, '|');
        if (data.size() >= 2)
        {
            nextOrderId = atoi(data[0].c_str());
            nextComplaintId = atoi(data[1].c_str());
        }
    }
}

void loadAllData(StudentList& students, vector<Dish>& dishes, vector<ClassInfo>& classes, vector<Complaint>& complaints, int& nextOrderId, int& nextComplaintId)
{
    bool hasStudents = loadStudentsFromFile(students);
    bool hasDishes = loadDishesFromFile(dishes);
    bool hasClasses = loadClassesFromFile(classes);

    if (!hasStudents) initStudents(students);
    if (!hasDishes) initDishes(dishes);
    if (!hasClasses) initClasses(classes);

    loadCountersFromFile(nextOrderId, nextComplaintId);
    loadOrdersFromFile(students, nextOrderId);
    loadComplaintsFromFile(complaints, nextComplaintId);
}

// ========================= 主函数 =========================

int main()
{


    StudentList students;//创建学生链表对象，负责管理学生对象的生命周期
    vector<Dish> dishes;//创建菜品向量对象，负责管理菜品对象的生命周期
    vector<ClassInfo> classes;//创建班级向量对象，保存班级信息
    vector<Complaint> complaints;//创建投诉建议向量对象，保存学生反馈
    int nextOrderId = 1;//订单编号从 1 开始，每次下单后自增
    int nextComplaintId = 1;//投诉建议编号从 1 开始，每次提交后自增
    loadAllData(students, dishes, classes, complaints, nextOrderId, nextComplaintId);//启动时优先读取文件，没有文件则使用初始化数据
    showDataFileHint();//提示数据文件的保存位置和作用
    mainMenu(students, dishes, nextOrderId, classes, complaints, nextComplaintId);//进入主菜单，传入学生链表、菜品向量和订单编号引用
    saveAllData(students, dishes, classes, complaints, nextOrderId, nextComplaintId);//退出系统时自动保存数据
    cout << "\n系统数据已经保存完成，可以在程序运行目录下查看 txt 数据文件。\n";
    return 0;
}

