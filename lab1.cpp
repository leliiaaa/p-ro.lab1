#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <algorithm>
#include <chrono>
#include <string>

using namespace std;

void threadFunc1() {
    cout << "1" << endl;
}

void threadFunc2() {
    cout << "2" << endl;
}

void runTask1_2_1() {
    cout << "\n--- Zavdannia 1.2.1 (bez join/detach - avariine zavershennia std::terminate) ---\n";
    cout << "Uvaha: Za standartom C++ znyshchennia std::thread bez join/detach vyklykaie terminate().\n";
    cout << "Demonstruiemo stvorennia potokiv:\n";
    try {
        thread t1(threadFunc1);
        thread t2(threadFunc2);
    } catch (...) {
        cout << "VynyInsight: Vynykle vykliuchennia cherez vidsutnist join/detach.\n";
    }
}

void runTask1_2_2() {
    cout << "\n--- Zavdannia 1.2.2 (z metodom detach) ---\n";
    thread t1(threadFunc1);
    thread t2(threadFunc2);
    t1.detach();
    t2.detach();
    this_thread::sleep_for(chrono::milliseconds(100));
}

list<int> globalList1;

void AddToList_Unsafe(int startVal) {
    for (int i = 0; i < 10; ++i) {
        globalList1.push_back(startVal + i);
        cout << "[AddToList] Dodano element: " << (startVal + i) << "\n";
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void ListContains_Unsafe(int targetVal) {
    for (int i = 0; i < 10; ++i) {
        auto it = find(globalList1.begin(), globalList1.end(), targetVal);
        if (it != globalList1.end()) {
            cout << "[ListContains] Element " << targetVal << " - VKHODYT\n";
        } else {
            cout << "[ListContains] Element " << targetVal << " - NE VKHODYT\n";
        }
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void runTask1_2_3() {
    cout << "\n--- Zavdannia 1.2.3 (Bez synkhronizatsii) ---\n";
    globalList1.clear();
    int target = 42;
    thread t1(AddToList_Unsafe, target);
    thread t2(ListContains_Unsafe, target);
    t1.join();
    t2.join();
}

list<int> globalList2;
mutex mtxList2;

void AddToList_Mutex(int startVal) {
    for (int i = 0; i < 10; ++i) {
        mtxList2.lock();
        globalList2.push_back(startVal + i);
        cout << "[AddToList_Mutex] Dodano: " << (startVal + i) << "\n";
        mtxList2.unlock();
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void ListContains_Mutex(int targetVal) {
    for (int i = 0; i < 10; ++i) {
        mtxList2.lock();
        auto it = find(globalList2.begin(), globalList2.end(), targetVal);
        if (it != globalList2.end()) {
            cout << "[ListContains_Mutex] Element " << targetVal << " - VKHODYT\n";
        } else {
            cout << "[ListContains_Mutex] Element " << targetVal << " - NE VKHODYT\n";
        }
        mtxList2.unlock();
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void runTask1_2_4() {
    cout << "\n--- Zavdannia 1.2.4 (Priame zastosuvannia mutex) ---\n";
    globalList2.clear();
    int target = 100;
    thread t1(AddToList_Mutex, target);
    thread t2(ListContains_Mutex, target);
    t1.join();
    t2.join();
}

list<int> globalList3;
mutex mtxList3;

void AddToList_Once(int val) {
    lock_guard<mutex> lock(mtxList3);
    globalList3.push_back(val);
    cout << "[AddToList_Once] Dodano: " << val << "\n";
}

void ListContains_Once(int targetVal) {
    lock_guard<mutex> lock(mtxList3);
    auto it = find(globalList3.begin(), globalList3.end(), targetVal);
    if (it != globalList3.end()) {
        cout << "[ListContains_Once] Poshuk " << targetVal << " -> VKHODYT\n";
    } else {
        cout << "[ListContains_Once] Poshuk " << targetVal << " -> NE VKHODYT\n";
    }
}

void runTask1_2_5() {
    cout << "\n--- Zavdannia 1.2.5 (std::lock_guard + detach 10 potokiv) ---\n";
    globalList3.clear();
    int baseVal = 50;

    for (int i = 0; i < 10; ++i) {
        thread tAdd(AddToList_Once, baseVal + i);
        thread tCheck(ListContains_Once, baseVal);
        tAdd.detach();
        tCheck.detach();
    }
    this_thread::sleep_for(chrono::milliseconds(200));
}

class someData {
public:
    string firstName;
    string lastName;
    string address;
    int age;

    someData(string f = "", string l = "", string a = "", int ag = 0)
        : firstName(f), lastName(l), address(a), age(ag) {}

    void print(const string& objName) const {
        cout << "[" << objName << "] " << firstName << " " << lastName
             << ", Adresa: " << address << ", Vik: " << age << "\n";
    }
};

class exchangePerson {
public:
    someData data;
    mutex mtx;

    exchangePerson(someData d) : data(d) {}

    static void JohnDoe(exchangePerson& p) {
        lock_guard<mutex> lock(p.mtx);
        p.data.firstName = "John";
        p.data.lastName = "Doe";
        p.data.address = "Unknown";
        p.data.age = 120;
    }

    static void JacobSmith(exchangePerson& p) {
        lock_guard<mutex> lock(p.mtx);
        p.data.firstName = "Jacob";
        p.data.lastName = "Smith";
        p.data.address = "Known";
        p.data.age = 1;
    }

    static void Swap_AdoptLock(exchangePerson& p1, exchangePerson& p2) {
        if (&p1 == &p2) return;

        cout << "\n--- Pered obminom (std::adopt_lock) ---\n";
        p1.data.print("Obiekt 1");
        p2.data.print("Obiekt 2");

        std::lock(p1.mtx, p2.mtx);
        lock_guard<mutex> lockA(p1.mtx, adopt_lock);
        lock_guard<mutex> lockB(p2.mtx, adopt_lock);

        std::swap(p1.data, p2.data);

        cout << "--- Pislia obminu (std::adopt_lock) ---\n";
        p1.data.print("Obiekt 1");
        p2.data.print("Obiekt 2");
    }

    static void Swap_DeferLock(exchangePerson& p1, exchangePerson& p2) {
        if (&p1 == &p2) return; 

        cout << "\n--- Pered obminom (std::defer_lock) ---\n";
        p1.data.print("Obiekt 1");
        p2.data.print("Obiekt 2");

        unique_lock<mutex> lockA(p1.mtx, defer_lock);
        unique_lock<mutex> lockB(p2.mtx, defer_lock);

        std::lock(lockA, lockB);

        std::swap(p1.data, p2.data);

        cout << "--- Pislia obminu (std::defer_lock) ---\n";
        p1.data.print("Obiekt 1");
        p2.data.print("Obiekt 2");
    }
};

void runTask1_2_6() {
    cout << "\n--- Zavdannia 1.2.6 (Swap: std::lock + std::adopt_lock) ---\n";
    exchangePerson person1(someData("Initial1", "Init1", "City1", 20));
    exchangePerson person2(someData("Initial2", "Init2", "City2", 25));

    thread t1(exchangePerson::JohnDoe, ref(person1));
    thread t2(exchangePerson::JacobSmith, ref(person2));
    t1.detach();
    t2.detach();

    this_thread::sleep_for(chrono::milliseconds(50));

    thread tSwap(exchangePerson::Swap_AdoptLock, ref(person1), ref(person2));
    tSwap.join();
}

void runTask1_2_7() {
    cout << "\n--- Zavdannia 1.2.7 (Swap: std::unique_lock + std::defer_lock) ---\n";
    exchangePerson person1(someData("Initial1", "Init1", "City1", 20));
    exchangePerson person2(someData("Initial2", "Init2", "City2", 25));

    thread t1(exchangePerson::JohnDoe, ref(person1));
    thread t2(exchangePerson::JacobSmith, ref(person2));
    t1.detach();
    t2.detach();

    this_thread::sleep_for(chrono::milliseconds(50));

    thread tSwap(exchangePerson::Swap_DeferLock, ref(person1), ref(person2));
    tSwap.join();
}

int main() {
    int choice = 0;
    do {
        cout << "\n1 - Zavdannia 1.2.1 (Potoky bez join/detach)\n";
        cout << "2 - Zavdannia 1.2.2 (Potoky z detach)\n";
        cout << "3 - Zavdannia 1.2.3 (Spysok bez synkhronizatsii)\n";
        cout << "4 - Zavdannia 1.2.4 (Synkhronizatsiia: mutex lock/unlock)\n";
        cout << "5 - Zavdannia 1.2.5 (10 potokiv + std::lock_guard)\n";
        cout << "6 - Zavdannia 1.2.6 (Swap: std::lock + adopt_lock)\n";
        cout << "7 - Zavdannia 1.2.7 (Swap: unique_lock + defer_lock)\n";
        cout << "0 - Vykhid\n";
        cout << "Oberit punkt: ";
        if (!(cin >> choice)) {
            break;
        }

        switch (choice) {
            case 1: runTask1_2_1(); break;
            case 2: runTask1_2_2(); break;
            case 3: runTask1_2_3(); break;
            case 4: runTask1_2_4(); break;
            case 5: runTask1_2_5(); break;
            case 6: runTask1_2_6(); break;
            case 7: runTask1_2_7(); break;
            case 0: cout << "Zavershennia roboty prohramy.\n"; break;
            default: cout << "Nevirnyi vybir. Sprobujte shche raz.\n";
        }
    } while (choice != 0);

    return 0;
}