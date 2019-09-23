#include <iostream>
#include <boost/cstdint.hpp>

inline const boost::uint_least32_t UnSynch(const boost::uint_least32_t & uValue)
{
    return (uValue & 0x7Fu) | ((uValue & (0x7Fu<<8u))>>1) | ((uValue & (0x7Fu<<16u))>>2) | ((uValue & (0x7Fu<<24u))>>3);
}
inline const boost::uint_least32_t Synch(const boost::uint_least32_t & uValue)
{
    return (uValue & 0x7Fu) | ((uValue & (0x7Fu<<7u))<<1) | ((uValue & (0x7Fu<<14u))<<2) | ((uValue & (0x7Fu<<21u))<<3);
}
inline const bool ValidSynch(const boost::uint_least32_t & uValue)
{
    return !(uValue & 0x80808080u);
}
int main()
{
    boost::uint_least32_t uValue = 0x0FFFFFFFu;
    std::cout << "Value: " << uValue << std::endl;
    uValue = Synch(uValue);
    std::cout << "Value: " << uValue << std::endl;
    uValue = UnSynch(uValue);
    std::cout << "Value: " << uValue << std::endl;
    return 0;
}

