#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

//Patent结构体：存储单条专利记录的完整信息包含专利ID、专利号、年份和新颖性得分四个字段
struct Patent {
    int id;
    string patentNumber;
    int year;
    double novelty;

    Patent(int i, const string& pn, int y, double n)
    : id(i), patentNumber(pn), year(y), novelty(n) {}

    Patent() : id(0), year(0), novelty(0.0) {}
};

// 单链表节点
struct Node {
    int id;
    string patentNumber;
    int year;
    double novelty;
    Node* next;

    Node(int i, const string& pn, int y, double n)
    : id(i), patentNumber(pn), year(y), novelty(n), next(nullptr) {}
};

//读取数据函数，从第500000开始读，只要2011-2020这十年的，每个年份只要读到1000条，就跳到下一年份
vector<Patent> initializeFromCSV(const string& filename, int maxRecords = 1000000) {
    vector<Patent> patents;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return patents;
    }

    string line;
    getline(file, line); // 跳过标题行

    cout << "开始读取CSV文件..." << endl;

    int recordsRead = 0;
    int totalLines = 0;
    int startLine = 500000; // 从第50000行开始读取，因为整体跑了一次，2011年的在500000条后一点，这样加快速度
    
    unordered_map<int, int> yearCount; // 记录每年已读取的数量
    for (int year = 2011; year <= 2020; year++) {
        yearCount[year] = 0;
    }

    // 跳过前面的行，直接到第500000行
    while (totalLines < startLine - 1 && getline(file, line)) {
        totalLines++;
    }

    cout << "已跳过前 " << startLine - 1 << " 行，从第 " << startLine << " 行开始读取" << endl;


//主读取循环
    while (getline(file, line) && recordsRead < maxRecords) {
        totalLines++;
        vector<string> tokens;
        string current;
        bool inQuotes = false;

        for (char c : line) {
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                tokens.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }

        tokens.push_back(current); // 添加最后一个字段

        // 数据提取和筛选，这部分大模型辅助，高效筛查
        if (tokens.size() >= 9) {
            try {
                int id = stoi(tokens[1]);    // ID字段（索引1）
                string patentNumber = tokens[2];    // PN字段（索引2）
                int year = stoi(tokens[6]);    // Year字段（索引6）
                double novelty = stod(tokens[8]);    // Novelty字段（索引8）

                // 只处理2011-2020年的数据，且每年不超过1000条
                if (year >= 2011 && year <= 2020 && yearCount[year] < 1000) {
                    patents.emplace_back(id, patentNumber, year, novelty);
                    yearCount[year]++;
                    recordsRead++;
                    
                    // 调试输出
                    if (recordsRead <= 3) {
                        cout << "成功读取: ID=" << id << ", PN=" << patentNumber
                             << ", Year=" << year << ", Novelty=" << novelty << endl;
                    }
                    
                    // 每读取100条输出一次进度
                    if (recordsRead % 100 == 0) {
                        cout << "已读取 " << recordsRead << " 条有效记录..." << endl;
                    }
                }
                
                // 如果已经收集够10000条记录，提前结束
                if (recordsRead >= 10000) {
                    break;
                }
            } catch (const exception& e) {
                // 忽略转换错误的行
                continue;
            }
        }
    }

    cout << "读取完成！从第 " << startLine << " 行开始，总共处理 " << (totalLines - startLine + 1) 
         << " 行，有效记录 " << recordsRead << " 条" << endl;
    
    // 输出每年的记录数量
    cout << "每年记录分布:" << endl;
    for (int year = 2011; year <= 2020; year++) {
        cout << year << "年: " << yearCount[year] << " 条" << endl;
    }
    
    file.close();
    return patents;
}

// 创建单链表：createLinkedList函数：将Patent向量转换为单链表。遍历所有专利记录，为每条记录创建链表节点，最后返回链表头指针
Node* createLinkedList(const vector<Patent>& patents) {
    if (patents.empty()) {
        cout << "警告：专利记录为空，无法创建链表" << endl;
        return nullptr;
    }

    Node* head = new Node(patents[0].id, patents[0].patentNumber, patents[0].year, patents[0].novelty);
    Node* current = head;

    for (size_t i = 1; i < patents.size(); ++i) {
        current->next = new Node(patents[i].id, patents[i].patentNumber, patents[i].year, patents[i].novelty);
        current = current->next;
    }
    
    cout << "链表创建完成，共 " << patents.size() << " 个节点" << endl;
    return head;
}

// 释放链表内存，这里也是大模型加上的，释放链表内存可以防止内存泄漏
void freeList(Node* head) {
    int count = 0;
    while (head) {
        Node* temp = head;
        head = head->next;
        delete temp;
        count++;
    }
    cout << "释放了 " << count << " 个节点内存" << endl;
}

// 打印链表信息
void printLinkedListInfo(Node* head) {
    if (!head) {
        cout << "链表为空" << endl;
        return;
    }
    
    int count = 0;
    Node* current = head;
    unordered_map<int, int> yearCount;
    
    while (current) {
        count++;
        yearCount[current->year]++;
        current = current->next;
    }
    
    cout << "链表节点总数: " << count << endl;
    cout << "链表中年份分布:" << endl;
    for (int year = 2011; year <= 2020; year++) {
        cout << year << "年: " << yearCount[year] << " 个节点" << endl;
    }
    
    // 显示前3个节点作为验证
    current = head;
    cout << "前3个节点样例:" << endl;
    for (int i = 0; i < 3 && current; i++) {
        cout << "节点" << i+1 << ": ID=" << current->id 
             << ", PN=" << current->patentNumber 
             << ", Year=" << current->year 
             << ", Novelty=" << current->novelty << endl;
        current = current->next;
    }
}



// 1、插入操作：新增一条2015年的专利记录，插入到对应年份的链表位置，insertPatent函数：在链表中插入新的专利记录，按年份顺序插入
Node* insertPatent(Node* head, int id, const string& patentNumber, int year, double novelty) {
    Node* newNode = new Node(id, patentNumber, year, novelty);
    
    // 如果链表为空，新节点作为头节点
    if (!head) {
        cout << "插入新专利: ID=" << id << ", PN=" << patentNumber 
             << ", Year=" << year << ", Novelty=" << novelty << endl;
        return newNode;
    }
    
    // 这是大模型建议的，虽然2015年一定不会在头部，但这样补充上会使边界更确切：如果新节点年份小于头节点年份，插入到链表头部
    if (year < head->year) {
        newNode->next = head;
        cout << "插入新专利: ID=" << id << ", PN=" << patentNumber 
             << ", Year=" << year << ", Novelty=" << novelty << " (插入到链表头部)" << endl;
        return newNode;
    }
    
    // 寻找插入位置：插入到对应年份组2015年的开始位置
    Node* current = head;
    Node* prev = nullptr;
    
    while (current && current->year < year) {
        prev = current;
        current = current->next;
    }
    
    // 插入节点
    if (prev) {
        prev->next = newNode;
    } else {
        head = newNode;
    }
    newNode->next = current;
    
    cout << "插入新专利: ID=" << id << ", PN=" << patentNumber 
         << ", Year=" << year << ", Novelty=" << novelty << endl;
    
    return head;//返回头结点
}


// 2、删除操作：删除新颖性得分 <0.3 的"低创新专利"节点，deleteLowNoveltyPatents函数：遍历链表，删除所有Novelty < 0.3的节点
Node* deleteLowNoveltyPatents(Node* head) {
    if (!head) {
        cout << "链表为空，无需删除" << endl;
        return nullptr;
    }
    
    Node* dummy = new Node(0, "", 0, 0.0); // 这步是大模型建议的，虚拟头节点，简化删除操作
    dummy->next = head;
    Node* prev = dummy;
    Node* current = head;
    int deleteCount = 0;
    
    while (current) {
        if (current->novelty < 0.3) {
            // 删除当前节点
            prev->next = current->next;
            cout << "删除低创新专利: ID=" << current->id 
                 << ", PN=" << current->patentNumber 
                 << ", Novelty=" << current->novelty << endl;
            delete current;
            current = prev->next;
            deleteCount++;
        } else {
            prev = current;
            current = current->next;//往下移动一个prev结点，再往下移动一个current结点
        }
    }
    
    Node* newHead = dummy->next;
    delete dummy;//删去虚拟链表头，回复真实链表头
    
    cout << "删除操作完成，共删除 " << deleteCount << " 个低创新专利节点" << endl;
    return newHead;
}


// 3、遍历操作：输出2018-2020年的所有专利号及新颖性得分，traverseRecentPatents函数：遍历并输出2018-2020年的专利信息
void traverseRecentPatents(Node* head) {
    if (!head) {
        cout << "链表为空，无法遍历" << endl;
        return;
    }
    
    cout << "=== 2018-2020年专利记录 ===" << endl;
    Node* current = head;
    int count = 0;
    bool found = false;
    
    while (current) {
        if (current->year >= 2018 && current->year <= 2020) {
            cout << "Year " << current->year << ": PN=" << current->patentNumber 
                 << ", Novelty=" << current->novelty << endl;
            count++;
            found = true;
        }
        current = current->next;
    }
    
    if (!found) {
        cout << "未找到2018-2020年的专利记录" << endl;
    } else {
        cout << "共找到 " << count << " 条2018-2020年的专利记录" << endl;
    }
}


// 4、查找操作：统计2016年专利的平均新颖性得分，calculateAverageNovelty函数：计算2016年的专利的平均新颖性得分
void calculateAverageNovelty(Node* head, int targetYear = 2016) {
    if (!head) {
        cout << "链表为空，无法计算平均值" << endl;
        return;
    }
    //初始化
    Node* current = head;
    double sum = 0.0;
    int count = 0;
    
    while (current) {
        if (current->year == targetYear) {
            sum += current->novelty;
            count++;
        }
        current = current->next;
    }
    
    if (count > 0) {
        double average = sum / count;
        cout << targetYear << "年专利新颖性统计:" << endl;
        cout << "专利数量: " << count << " 条" << endl;
        cout << "新颖性总分: " << sum << endl;
        cout << "平均新颖性得分: " << average << endl;
    } else {
        cout << "未找到" << targetYear << "年的专利记录" << endl;
    }
}

// 主函数，实现所有函数的调用
int main() {
    string filename = "E:\\InnovationDataset\\DeepInnovationAI\\DeepPatentAI.csv";
    
    // 1. 读取CSV数据
    cout << "=== 阶段1：数据读取 ===" << endl;
    vector<Patent> patents = initializeFromCSV(filename, 10000);
    cout << "成功加载 " << patents.size() << " 条专利记录" << endl;
    
    // 2. 链表构建
    cout << "\n=== 阶段2：链表构建 ===" << endl;
    Node* head = createLinkedList(patents);
    
    // 3. 结果验证
    cout << "\n=== 阶段3：结果验证 ===" << endl;
    if (head) {
        printLinkedListInfo(head);
    }
    
    // ============================================================================
    // 新增功能测试
    // ============================================================================
    
    cout << "\n=== 阶段4：链表操作测试 ===" << endl;
    
    // 4.1 插入操作：新增2015年专利记录
    cout << "\n--- 插入操作 ---" << endl;
    head = insertPatent(head, 999999, "US2024000001", 2015, 0.85);
    
    // 验证插入后的链表
    cout << "插入后链表信息:" << endl;
    printLinkedListInfo(head);
    
    // 4.2 删除操作：删除低创新专利
    cout << "\n--- 删除操作 ---" << endl;
    head = deleteLowNoveltyPatents(head);
    
    // 验证删除后的链表
    cout << "删除后链表信息:" << endl;
    printLinkedListInfo(head);
    
    // 4.3 遍历操作：输出2018-2020年专利
    cout << "\n--- 遍历操作 ---" << endl;
    traverseRecentPatents(head);
    
    // 4.4 查找操作：统计2016年平均新颖性
    cout << "\n--- 查找操作 ---" << endl;
    calculateAverageNovelty(head, 2016);
    
    // 5. 内存清理
    cout << "\n=== 阶段5：内存清理 ===" << endl;
    freeList(head);
    
    cout << "\n程序执行完成！" << endl;
    return 0;
}
