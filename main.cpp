#include <iostream>
#include <memory>
#include <functional>
#include <cstring>
#include <algorithm>
#include <vector>

// #include "src/server.h"
using namespace std;
template <typename _T>
class S
{
public:
    S(int f) : _m(f){};
    void f(int x)
    {
        _m += x;
    }
    friend ostream &operator<<(ostream &out, const S &s)
    {
        out << s._m << endl;
        return out;
    }

private:
    int _m;
};
using Func = void(int);

void callback(function<void(int)> f)
{
    f(2);
}
template <typename _T>
void add1(_T &&i)
{
    // i++;
    typename remove_reference<_T>::type &&m = forward<_T &&>(2);
    cout << m << endl;
    //const_cast<typename remove_reference<_T>::type >(m)++;
}
class Base
{
private:
    string name;

public:
    Base(string &&name) : name(name)
    {
        memset(this, 0, sizeof(*this));
    };
    virtual void show()
    {
        cout << "Base show()" << endl;
    }
    virtual void show(bool x)
    {
        cout << "Base: " << ios::boolalpha << x << endl;
    }
    virtual void show(double x)
    {
        cout << "Base: " << x << endl;
    }
    virtual ~Base()
    {
        cout << "Base ~" << endl;
    }
};
class Students : public Base
{
public:
    Students(string &&s) : Base(move(s)){};
    virtual void show() override
    {
        cout << "Students: show" << 9999 << endl;
    }
    void show(bool x) override
    {
        cout << "Students: bool";
        cout.setf(ios::boolalpha);
        cout << x << endl;
    }
    virtual void show(double x) override
    {
        cout << "Students: double" << x << endl;
    }
    // ----------
};

typedef void (*Func1)();
typedef void (*FuncWD)(double);

void call(shared_ptr<Base> pb)
{
    cout << "use count" << pb.use_count() << endl;
    pb->show();
    pb->show(true);
}

class Obj
{
public:
    // virtual ~Obj() = default;
    virtual bool operator<(const Obj &r)
    {
        return this->value() < r.value();
    }
    friend bool operator<(const Obj &lv, const Obj &rv);
    virtual int value() const = 0;
    virtual Obj *clone() const = 0;
    virtual ostream &operator>>(ostream &out) const = 0;
};

class IntObj : public Obj
{
private:
    int _i;

public:
    IntObj(int i) : _i(i){};
    virtual int value() const override { return _i; }
    virtual IntObj *clone() const override
    {
        return new IntObj(_i);
    }
    virtual ostream &operator>>(ostream &out) const
    {
        out << _i;
        return out;
    }
};

class StrObj : public Obj
{
private:
    string _str;

public:
    StrObj(string str) : _str(str){};
    virtual int value() const override { return static_cast<int>(_str[0]); }
    virtual StrObj *clone() const override //return value as point and type is self
    {
        return new StrObj(_str);
    }
    virtual ostream &operator>>(ostream &out) const
    {
        out << _str;
        return out;
    }
};

class StuObj : public Obj
{
private:
    int id;
    string name;

public:
    StuObj(int id, string name) : id(id), name(name){};
    virtual int value() const override { return id; }
    virtual StuObj *clone() const override //return value as point and type is self
    {
        return new StuObj(id, name);
    }
    virtual ostream &operator>>(ostream &out) const
    {
        out << "{ " << id << ", " << name << " }";
        return out;
    }

    // friend ostream &operator<<(ostream &out, const StuObj &obj);
};

class List;
class Node
{
    friend List;

public:
    Node(const shared_ptr<Obj> &obj) : data(obj), next(nullptr){};
    friend ostream &operator<<(ostream &out, const Node &node);

private:
    shared_ptr<Obj> data;
    Node *next;
};
ostream &operator<<(ostream &out, const Node &node)
{
    (*node.data) >> out;
    // node.data->operator<<(out);
    return out;
}
class List
{
private:
    Node *head = nullptr, *tail = nullptr;
    int size = 0;

public:
    List() = default;
    ~List()
    {
        while (head)
        {
            Node *tmp = head->next;
            delete head;
            head = tmp;
        }
    }
    List(const List &other)
    {
        Node *tmp = other.head;
        while (tmp != nullptr)
        {
            // add(shared_ptr<Obj>(tmp->data->clone()));
            add(tmp->data);
            tmp = tmp->next;
        }
    }
    void add(const shared_ptr<Obj> &obj)
    {
        Node *n = new Node(obj);
        if (head)
        {
            tail->next = n;
            tail = n;
        }
        else
        {
            tail = n, head = n;
        }
        size++;
    }

    void print() const
    {
        Node *tmpNode = head;
        while (tmpNode)
        {
            (*tmpNode->data) >> cout;
            tmpNode = tmpNode->next;
            if (tmpNode)
                cout << "->";
        }
        cout << endl;
    }

    void sort()
    {
        if (size == 0)
            return;
        int i = 0;
        Node *p = head;
        vector<Node *> ver(size);
        while (p != nullptr)
        {
            ver[i++] = p;
            p = p->next;
        }
        std::sort(ver.begin(), ver.end(), [](Node *l, Node *r) -> bool { return (*l->data) < (*r->data); });
        tail = head = ver[0];
        for (i = 1; i < size; i++)
        {
            tail->next = ver[i];
            tail = ver[i];
            tail->next = nullptr;
        }

        // for (; p != nullptr; p = p->next)
        // {
        //     for (q = p; q->next != nullptr; q = q->next)
        //     {
        //         if ((*p->data) < (*q->data))
        //         {
        //             // std::swap(p->data, q->data);
        //             p->data.swap(q->data);
        //             // swap(p->data, );
        //         }
        //     }
        // }
    }
};

bool operator<(const Obj &lv, const Obj &rv)
{
    return lv.value() < rv.value();
}
int main()
{
    /*
        decltype(make_shared<ServerHandle>) pserver = make_shared<ServerHandle>();
        shared_ptr<Base> p = make_shared<Students>("test");
        long long *p_vtabl = reinterpret_cast<long long *>(p.get());
        long long *vtabl = reinterpret_cast<long long *>(*p_vtabl);
        Func1 vtpf1 = reinterpret_cast<Func1>(vtabl[0]);
        FuncWD vtpf2 = reinterpret_cast<FuncWD>(vtabl[1]);
        Func1 vtpf1deac = reinterpret_cast<Func1>(vtabl[2]);
        vtpf1();
        vtpf1deac();
        cout << typeid(*p).name() << endl;
        vtpf2(3.5);
        p->show();
        call(p);
        p->show();
        pserver->Start();
        cout << i << endl;
    */
    List l;
    auto ps = make_shared<StrObj>("daxie");
    l.add(make_shared<StrObj>("nihao"));
    l.add(make_shared<StuObj>(19201080, "zhong"));
    l.add(make_shared<IntObj>(-5));
    l.print();
    l.add(ps);
    l.sort();
    l.print();
    List l2(l);
    l2.print();
    return 0;
}
