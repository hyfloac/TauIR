#include <cstdio>

template<typename Derived>
class BaseVisitor
{
public:
    Derived& GetDerived() noexcept { return *static_cast<Derived*>(this); }

    void TraverseNop()
    {
        GetDerived().VisitNop();
    }

    void TraverseLabel(int label)
    {
        GetDerived().VisitLabel(label);
    }

    void Traverse(const unsigned char* const buffer, int length)
    {
        for(int i = 0; i < length; ++i)
        {
            if(buffer[i] == 0x00)
            {
                TraverseNop();
            }
            else if(buffer[i] == 0x01)
            {
                int var = *reinterpret_cast<const int*>(buffer + i + 1);
                i += sizeof(int);
                TraverseLabel(var);
            }
        }
    }

    void VisitNop() { }
    void VisitLabel(int label) { }
};

class VisitorNop : public BaseVisitor<VisitorNop>
{
public:
    void VisitNop()
    {
        (void) printf("Visited Nop.\n");
    }
};

class VisitorLabel : public BaseVisitor<VisitorLabel>
{
public:
    void VisitLabel(int label)
    {
        (void) printf("Visited label 0x%x.\n", label);
    }
};

int main()
{
    const unsigned char buffer[] = { 0x00, 0x00, 0x01, 0x78, 0x56, 0x34, 0x12, 0x00 };

    {
        VisitorNop visitor;
        visitor.Traverse(buffer, sizeof(buffer));
    }

    {
        VisitorLabel visitor;
        visitor.Traverse(buffer, sizeof(buffer));
    }

    return 0;
}
