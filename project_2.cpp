/*
 * 校园学生食堂信息管理系统（project_2 精简增强版）
 * ------------------------------------------------
 * 设计目标：
 * 1. 使用“学生类链表”保存学生信息，体现链表插入、删除、查找、遍历。
 * 2. 管理员作为超级用户，通过“全局函数”管理学生、菜品并查询订单。
 * 3. 学生登录后，通过“学生对象自己的成员函数”完成订餐、支付、取餐、评价、投诉建议。
 * 4. 管理员可以查看统计报表、热销排行榜、菜品评价和投诉建议。
 * 5. 菜品使用结构体数组 vector 保存，代码难度适合大一学生阅读和汇报。
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
const int SCREEN_WIDTH = 96;        // 控制台分隔线宽度，仅用于美化显示

// 订单状态：
enum class OrderStatus //枚举类型，作用域调用，判断订单处于哪一个业务阶段
{
    Reserved,          // 待支付
    Paid,              // 已支付，等待取餐
    PickedUp,          // 已取餐
    Cancelled,         // 已取消
    Refunded           // 已退款
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
    vector<string> comments;// 学生评价留言
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
    string payMethod;       // 支付方式：未支付 / 校园卡 / 微信 / 支付宝 / 已退款
    bool rated;             // 是否已经评价，避免同一订单重复评分
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

bool shouldCountRevenue(OrderStatus status) //统计营收时，只统计已经支付或已经取餐的订单
{
    return status == OrderStatus::Paid || status == OrderStatus::PickedUp;
}

bool shouldCountSales(OrderStatus status) //热销排行只统计真实有效的订单数量
{
    return status == OrderStatus::Paid || status == OrderStatus::PickedUp;
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
    cout << "菜品列表如下：\n";
}

void showDishBrief(const Dish& dish) // 打印单个菜品简略信息函数
{
    printLine('-');
    cout << "编号：" << dish.id << "    名称：" << dish.name << '\n';
    cout << "分类：" << dish.category << "    价格：" << fixed << setprecision(2) << dish.price << " 元";
    cout << "    库存：" << dish.stock << " 份";
    if (dish.stock <= LOW_STOCK_LINE) //LOW_STOCK_LINE为全局常量，表示库存低于等于该值时提示库存偏低
    {
        cout << "（库存偏低）";
    }
    cout << '\n';
    cout << "口味：" << dish.flavor << "    评分：" << fixed << setprecision(1) << dish.rating
         << " 星（" << dish.ratingCount << " 人评价）";
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
    cout << "评分：" << fixed << setprecision(1) << dish.rating << " 星（" << dish.ratingCount << " 人评价）\n";
    if (dish.comments.empty())
    {
        cout << "评价留言：暂无\n";
    }
    else
    {
        cout << "评价留言：\n";
        for (const string& comment : dish.comments)
        {
            cout << " - " << comment << '\n';
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

    showDishTableHeader();
    for (const Dish& dish : dishes) 
	{
        showDishBrief(dish);//打印每个菜品的简略信息
    }
    printLine('-');
}

void searchAndFilterDishes(const vector<Dish>& dishes) //菜品搜索和筛选函数
{
    if (dishes.empty())
    {
        cout << "当前没有菜品信息。\n";
        return;
    }

    cout << "\n【菜品搜索与筛选】\n";
    cout << "1. 按关键字搜索（名称/分类/口味）\n";
    cout << "2. 按分类筛选\n";
    cout << "3. 按价格区间筛选\n";
    cout << "4. 只看有库存菜品\n";
    int choice = readInt("请选择：", 1, 4);

    string keyword;
    double minPrice = 0.0;
    double maxPrice = 0.0;
    if (choice == 1)
    {
        keyword = readText("请输入关键字：");
    }
    else if (choice == 2)
    {
        keyword = readText("请输入分类（如 早餐/午餐/晚餐/特色菜）：");
    }
    else if (choice == 3)
    {
        minPrice = readDouble("请输入最低价格：", 0.0, 9999.0);
        maxPrice = readDouble("请输入最高价格：", minPrice, 9999.0);
    }

    bool found = false;
    showDishTableHeader();
    for (const Dish& dish : dishes)
    {
        bool match = false;
        if (choice == 1)
        {
            match = dish.name.find(keyword) != string::npos
                 || dish.category.find(keyword) != string::npos
                 || dish.flavor.find(keyword) != string::npos;
        }
        else if (choice == 2)
        {
            match = dish.category == keyword;
        }
        else if (choice == 3)
        {
            match = dish.price >= minPrice && dish.price <= maxPrice;
        }
        else if (choice == 4)
        {
            match = dish.stock > 0;
        }

        if (match)
        {
            showDishBrief(dish);
            found = true;
        }
    }
    printLine('-');

    if (!found)
    {
        cout << "没有找到符合条件的菜品。\n";
    }
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

    string getId() const { return studentId; } //const保护对象成员
    string getName() const { return name; }
    string getGrade() const { return grade; }
    string getMajor() const { return major; }
    const vector<Order>& getOrders() const { return orders; } // 返回订单向量的常量引用，避免拷贝，提高效率

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
        string newPhone = readText("联系电话（当前：" + phone + "）：", true);

        if (!newName.empty()) name = newName; //如果输入不为空，则更新学生信息
        if (!newGender.empty()) gender = newGender;
        if (!newBirthDate.empty()) birthDate = newBirthDate;
        if (!newGrade.empty()) grade = newGrade;
        if (!newMajor.empty()) major = newMajor;
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
            if (index == -1) 
            {
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
        newOrder.payMethod = "未支付";
        newOrder.rated = false;

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
        if (orders.empty())
        {
            cout << "你目前没有订单，暂时不能评价菜品。\n";
            return;
        }

        cout << "只有已取餐且未评价的订单可以评价。\n";
        showOrders();
        int orderId = readInt("请输入要评价的订单编号：", 1, 999999);
        for (Order& order : orders)
        {
            if (order.orderId == orderId)
            {
                if (order.status != OrderStatus::PickedUp)
                {
                    cout << "该订单还没有取餐，不能评价。\n";
                    return;
                }
                if (order.rated)
                {
                    cout << "该订单已经评价过，不能重复评价。\n";
                    return;
                }

                int index = findDishIndexById(dishes, order.dishId);
                if (index == -1)
                {
                    cout << "该菜品已经不存在，暂时不能评价。\n";
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
                order.rated = true;

                cout << "评价成功，菜品【" << dish.name << "】当前平均分为 "
                     << fixed << setprecision(1) << dish.rating << " 星。\n";
                return;
            }
        }
        cout << "没有找到该订单。\n";
    }

    void submitComplaint(vector<Complaint>& complaints, int& nextComplaintId) //学生提交投诉建议函数
    {
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

    void viewMyComplaints(const vector<Complaint>& complaints) const //查看自己的投诉建议处理情况
    {
        bool found = false;
        printLine('-');
        cout << "我的投诉建议记录：\n";
        for (const Complaint& complaint : complaints)
        {
            if (complaint.studentId == studentId)
            {
                found = true;
                printLine('-');
                cout << "编号：" << complaint.complaintId << '\n';
                cout << "类型：" << complaint.type << '\n';
                cout << "内容：" << complaint.content << '\n';
                cout << "状态：" << (complaint.handled ? "已处理" : "未处理") << '\n';
                cout << "管理员回复：" << complaint.reply << '\n';
            }
        }
        if (!found)
        {
            cout << "你还没有提交过投诉建议。\n";
        }
        printLine('-');
    }

    void showOrders() const //打印当前学生对象的所有订单简略信息
    {
        // 成员函数：只显示当前学生自己的订单。
        if (orders.empty()) 
        {
            cout << "当前没有订单记录。\n";
            return;
        }

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
            cout << "评价：" << (order.rated ? "已评价" : "未评价") << '\n';
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
    students.addStudent(new Student("20260001", "111111", "李明", "男", "2007-03-12", "大一", "软件工程", "13800000001"));
    students.addStudent(new Student("20260002", "222222", "王雨", "女", "2007-07-05", "大一", "计算机科学", "13800000002"));
    students.addStudent(new Student("20260003", "333333", "张辰", "男", "2006-11-20", "大二", "网络工程", "13800000003"));
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
    string phone = readText("请输入联系电话：");
    return new Student(id, password, name, gender, birthDate, grade, major, phone);//在堆上创建学生对象，返回指针
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
        cout << "5. 搜索/筛选菜品\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 5);
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
            case 5:
                searchAndFilterDishes(dishes);//搜索筛选菜品
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
            cout << "订单号：" << order.orderId << "    菜品：" << order.dishName << " × " << order.quantity << '\n';
            cout << "金额：" << fixed << setprecision(2) << order.amount << " 元"
                 << "    支付：" << order.payMethod
                 << "    状态：" << orderStatusToString(order.status) << '\n';
            cout << "时段：" << order.mealTime << "    方式：" << order.mode << "    取餐码：" << order.pickupCode << '\n';
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
    if (student == nullptr) 
    {
        cout << "没有找到该学生。\n";
        return;
    }

    cout << "学生【" << student->getName() << "】的订单信息：\n";
    student->showOrders();
}

void orderQueryMenu(const StudentList& students) //管理员订单查询菜单函数
{
    while (true) 
    {
        printLine();
        cout << "管理员菜单 > 订单查询\n";
        printLine();
        cout << "1. 查看全部订单\n";
        cout << "2. 按学生查询订单\n";
        cout << "3. 按菜品名称查询订单\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 3);
        if (choice == 0) return;

        switch (choice) 
        {
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

// ========================= 管理员全局函数：数据统计和热销排行 =========================

void showHotDishRanking(const StudentList& students, int topCount = 5) //热销菜品排行榜
{
    vector<int> dishIds;
    vector<string> dishNames;
    vector<int> dishQuantities;

    Student* current = students.getHead();
    while (current != nullptr)
    {
        const vector<Order>& orders = current->getOrders();
        for (const Order& order : orders)
        {
            if (!shouldCountSales(order.status))
            {
                continue;
            }

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

    if (dishIds.empty())
    {
        cout << "暂无已支付或已取餐订单，无法生成热销排行榜。\n";
        return;
    }

    for (size_t i = 0; i < dishIds.size(); ++i)
    {
        for (size_t j = i + 1; j < dishIds.size(); ++j)
        {
            if (dishQuantities[j] > dishQuantities[i])
            {
                swap(dishIds[i], dishIds[j]);
                swap(dishNames[i], dishNames[j]);
                swap(dishQuantities[i], dishQuantities[j]);
            }
        }
    }

    printLine('-');
    cout << "热销菜品排行榜（按有效销量统计）\n";
    printLine('-');
    int rows = min(topCount, static_cast<int>(dishIds.size()));
    for (int i = 0; i < rows; ++i)
    {
        cout << i + 1 << ". " << dishNames[i]
             << "（编号：" << dishIds[i] << "），累计销售 "
             << dishQuantities[i] << " 份\n";
    }
    printLine('-');
}

void adminDataAnalysis(const StudentList& students) //管理员数据统计分析函数
{
    int totalStudents = students.countStudents();
    int totalOrders = 0;
    int reservedCount = 0, paidCount = 0, pickedCount = 0, cancelledCount = 0, refundedCount = 0;
    int breakfastCount = 0, lunchCount = 0, dinnerCount = 0;
    int dineInCount = 0, takeAwayCount = 0, deliveryCount = 0;
    int cardPayCount = 0, wechatPayCount = 0, alipayCount = 0;
    double totalRevenue = 0.0;

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
        }
        current = current->next;
    }

    printLine('-');
    cout << "学生总人数：" << totalStudents << '\n';
    cout << "订单总数：" << totalOrders << '\n';
    cout << "待支付：" << reservedCount << "，已支付：" << paidCount
         << "，已取餐：" << pickedCount << "，已取消：" << cancelledCount
         << "，已退款：" << refundedCount << '\n';
    cout << "有效营收：" << fixed << setprecision(2) << totalRevenue << " 元\n";
    cout << "用餐时段统计：早餐 " << breakfastCount << "，午餐 " << lunchCount << "，晚餐 " << dinnerCount << '\n';
    cout << "用餐方式统计：堂食 " << dineInCount << "，自提 " << takeAwayCount << "，外卖 " << deliveryCount << '\n';
    cout << "支付方式统计：校园卡 " << cardPayCount << "，微信 " << wechatPayCount << "，支付宝 " << alipayCount << '\n';
    printLine('-');
    showHotDishRanking(students, 3);
}

void dataAnalysisMenu(const StudentList& students) //管理员数据统计菜单函数
{
    while (true)
    {
        printLine();
        cout << "管理员菜单 > 数据统计分析\n";
        printLine();
        cout << "1. 查看综合统计报表\n";
        cout << "2. 查看热销菜品排行榜\n";
        cout << "0. 返回上一级\n";

        int choice = readInt("请选择：", 0, 2);
        if (choice == 0) return;

        if (choice == 1) adminDataAnalysis(students);
        if (choice == 2) showHotDishRanking(students);
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
    } 
    else 
    {
        cout << "注册失败：该学号已经存在。\n";
        delete newStudent;
    }
}

void adminMenu(StudentList& students, vector<Dish>& dishes, vector<Complaint>& complaints) //管理员菜单函数
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
        cout << "5. 查看菜品评价\n";
        cout << "6. 投诉建议处理\n";
        cout << "0. 退出管理员菜单\n";

        int choice = readInt("请选择：", 0, 6);
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
                adminShowDishComments(dishes);//管理员查看菜品评价
                pauseScreen();
                break;
            case 6:
                complaintManageMenu(complaints);//管理员投诉建议处理菜单
                break;
        }
    }
}

void studentMenu(Student* student, vector<Dish>& dishes, int& nextOrderId, vector<Complaint>& complaints, int& nextComplaintId, const StudentList& students) //学生菜单函数
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
        cout << "10. 查看我的投诉回复\n";
        cout << "11. 取消订单/退款\n";
        cout << "12. 搜索/筛选菜品\n";
        cout << "13. 查看热销排行榜\n";
        cout << "0. 退出学生菜单\n";

        int choice = readInt("请选择：", 0, 13);
        if (choice == 0) return;

        switch (choice) 
        {
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
                student->viewMyComplaints(complaints);
                break;
            case 11:
                student->cancelOrRefundOrder(dishes);
                break;
            case 12:
                searchAndFilterDishes(dishes);
                break;
            case 13:
                showHotDishRanking(students);
                break;
        }
        pauseScreen();
    }
}

void mainMenu(StudentList& students, vector<Dish>& dishes, int& nextOrderId, vector<Complaint>& complaints, int& nextComplaintId) //主菜单函数
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
                    adminMenu(students, dishes, complaints);//管理员菜单函数
                }
                pauseScreen();
                break;
            case 2: 
            {
                Student* student = studentLogin(students);//如果学生登录成功，则进入学生菜单
                if (student != nullptr) 
                {
                    studentMenu(student, dishes, nextOrderId, complaints, nextComplaintId, students);//学生菜单函数
                }
                pauseScreen();
                break;
            }
            case 3:
                registerStudent(students);//学生注册函数，参数是学生链表引用
                pauseScreen();
                break;
        }
    }
}

// ========================= 主函数 =========================

int main()
{
    StudentList students;//创建学生链表对象，负责管理学生对象的生命周期
    vector<Dish> dishes;//创建菜品向量对象，负责管理菜品对象的生命周期
    vector<Complaint> complaints;//投诉建议列表，管理员统一查看和回复
    int nextOrderId = 1;//订单编号从 1 开始，每次下单后自增
    int nextComplaintId = 1;//投诉建议编号从 1 开始，每次提交后自增
    initStudents(students);//初始化学生数据
    initDishes(dishes);//初始化菜品数据
    mainMenu(students, dishes, nextOrderId, complaints, nextComplaintId);//进入主菜单
  
}

