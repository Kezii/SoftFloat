#include <iostream>
#include <bitset>
#include <cassert>
#include <string>
#include <stdint.h>
#include <chrono>


using namespace std;


class kloat{
    union data{
        float fl;
        struct { //warning: little endian
            unsigned int mantissa: 23;
            unsigned int exp:8;
            unsigned int sign:1;
        } bits;
    };

public:

#define unbias_exp(v) (v - 127)

    data number;

    kloat() {};

    kloat(float n) {
        set(n);
    }

    void set(float n){
        number.fl=n;
    }

    void info(std::string name=""){
        int sign = number.bits.sign;
        int exp = number.bits.exp;
        int m = number.bits.mantissa;

        cout << endl << "---------------" << name  << endl;
        cout << "value:    "  << number.fl << endl;
        cout << "sign:     "  << std::bitset<1>(sign) << endl;
        cout << "exponent: " << unbias_exp(exp)  << " " << std::bitset<8>(exp) << endl;
        cout << "mantissa: " << m  << " " << std::bitset<23>(m) << endl;
        cout << "---------------" << endl;

    }

    void print(){
        cout << std::bitset<1>(number.bits.sign) << " " << std::bitset<8>(number.bits.exp) << " " << std::bitset<23>(number.bits.mantissa) << endl;
        cout << number.bits.sign << " " << number.bits.exp << " " <<number.bits.mantissa << endl;
    }

    kloat& operator = (kloat n){
        number= n.number;
        return *this;
    }

    operator float() {
        return number.fl;
    }

    kloat& operator + (kloat other){

        kloat ret;

        if(other == kloat(0)){
            return *this;
        }

        if(*this == kloat(0)){
            return other;
        }

        auto x = number.bits;
        auto y = other.number.bits;


        uint32_t xm, ym;

        xm = (x.mantissa >> 1) + 0x400000; //add the missing 1, 23° bit
        ym = (y.mantissa >> 1) + 0x400000;


        int max_exp;
        if(x.exp > y.exp){
            ym = (ym >> (x.exp - y.exp));
            max_exp = x.exp;
        }else{
            max_exp = y.exp;
            xm = (xm >> (y.exp - x.exp));
        }

        uint32_t zm;

        int xsign, ysign;
        xsign=(x.sign==1)?-1:1;
        ysign=(y.sign==1)?-1:1;


        if(number.bits.sign == other.number.bits.sign ){
            zm = xm + ym;
            ret.number.bits.sign = number.bits.sign;
        }else{

            if(xm > ym){
                zm = xm - ym;
                ret.number.bits.sign = x.sign;
            }
            else {
                zm = ym - xm;
                ret.number.bits.sign = y.sign;
            }
        }

        auto first_bit = 31 - __builtin_clz(zm);

        int h = first_bit - 22;

        if(h>=0)
            ret.number.bits.mantissa = (zm << 1) >> h;
        else
            ret.number.bits.mantissa = (zm << 1) << -h;

        ret.number.bits.exp = max_exp + h;

        return ret;
    }


    kloat& operator - (kloat other){
        other.number.bits.sign=!other.number.bits.sign;
        return *this + other;
    }

    kloat& operator += (kloat other){
        *this = *this + other;
        return *this;
    }

    kloat& operator -= (kloat other){
        *this = *this - other;
        return *this;
    }

    bool operator== (kloat other){
        return (number.fl==other.number.fl);
    }
};



using namespace std::chrono ;

int main(int argc, char *argv[])
{

    // random tests

    kloat lol;
    float asd;

    lol=-100.f;
    asd=-100.f;


    auto start = steady_clock::now() ;

    for(int i=0;i<1000;i++){
        lol+=kloat(i);
        asd+=i;
    }

    assert(asd==lol.number.fl);


    lol=0.f;

    asd=0.f;


    for(int i=-30;i<1000;i++){
        lol+=kloat(i);
        asd+=i;
    }
    assert(asd==lol.number.fl && 1);

    lol=0;

    for(int i=-15000000;i<15000000;i++){
        lol+=i;
    }


    lol=0;
    cout << lol - kloat(10.f) << endl;

    auto end = steady_clock::now() ;

    std::cout  << duration_cast<milliseconds>(end-start).count()  << " milliseconds\n" ;

    return 0;
}
