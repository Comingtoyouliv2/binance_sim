#pragma once // Only to be #inlcude onece

#include <cstdint>
#include <memory>
#include <vector> 

class Order;

using Price = std::int32_t; // double, float 쓰지 않는 이유: 부동소수점 연산은 
                            // 정확도가 떨어지기 때문에, 가격 계산에서 오차가 생길 수 있다.
                            // 대신에, 가격을 정수로 표현하기 위해, 예를 들어, 
                            // 1 단위가 0.000001 달러가 되도록 한다.
                            // 오류 예시 
                            // doube price = 150.50;
                            // price * 0.01 = 150.50999999999... 
using Quantity = std::uint32_t;
using OrderId = std::uint32_t;
using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::vector<OrderPointer>;