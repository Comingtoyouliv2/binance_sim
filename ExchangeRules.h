#pragma once

#include "types.h"

struct ExchangeRules {
    Price tickSize = 1; // 가격의 최소 변동 단위, 42000.001234 처럼 무한 소수로 쪼개지지 않게 단위 통일
    Quantity lotSize = 1; // 주문 수량의 최소 변동단위 => 이걸 차후에는 1 satoshi = 0.000001 BTC 이런 느낌으로
                            // 나눠서 비트코인과 같은 high price stock/coin 도 거래 가능하게 만든다. 
    Quantity minQuantity = 1; // 수량이 0.000001 처럼 너무 작은건 걸러버리는 목적, 최소 거래량
    Quantity maxQuantity = 1000000; // 오류성 거래 보호 목적으로 최대 주문 수량을 제한 
    Price minNotional = 0; // Price * Quantity 가 최소 얼마 이상이어야 하는지 알려준다.

    // 가격이 음수면 valid order 로 처리 x
    // tickSize = 1 이므로 나머지는 항상 0이 나온다. 그러므로 어떤 가격이든 허용해놓음
    bool IsValidPrice(Price price) const {
        if (price <= 0) {
            return false;
        }
        return price % tickSize == 0 ;
    }

    bool IsValidQuantity(Quantity quantity) const {
        if (quantity < minQuantity || quantity > maxQuantity) {return false;}
        return quantity % lotSize == 0;
    }

    // 1. notional 이 64_t 인 이유: price (32bit) * quantity (32 bit) = 64 bit 이므로, int64_t 로 표현해야 overflow 안 난다.
    // 2. 단순 타입 변환이 아니라 static_cast<type> 를 사용한 이유
    //    2-1 (int64_t) price 이러면, type table 확인 안하고, 그냥 비트만 재해석, 
    //       문제는 (int64_t*) price 라면, (when price is int* = 0x7fff5abc 라고 가정)
    //      (int64_t*) price -> 0x000000007fff5abc (64 bit 로 재해석, 앞에 0이 붙는다. 
    //      이러면 완전히 다른 메모리 주소가 되어 다른 곳을 가르킨다. 
    //      현재는 단순한 int 라 괜찮지만, static_cast 를 사용하는 습관을 들이자 
    bool IsValidNotional(Price price, Quantity quantity) const {
        int64_t notional = static_cast<int64_t>(price) * static_cast<int64_t>(quantity);
        return notional >= minNotional;
    }

    bool IsValidOrder(Price price, Quantity quantity) const {
        return IsValidPrice(price) && 
               IsValidQuantity(quantity) && 
               IsValidNotional(price, quantity);
    }
    
    // 가격을 tick size 단위로 반올림, 내림 하는 함수
    // 일단은 tickSize = 1 로 설정되어있기에, 차후에 100 처럼 단위가 변경되면 사용
    // e.g. if tick size = 100, then $150.73 -> Invalid Price
    // RoundToTick(150.73) ->  15073 / 100 = 150 -> 150 * 100 = 15000 (= $150.00)
    // 이렇게 tick 별로 범위를 나눠서 호가창을 관리한다. 안그러면 무한하게 파편화된 호가창이 생긴다. 
    // 실제로는: Binance tickSize => $0.01, NYSE => tickSize = $0.01 이다. 
    Price RoundToTick(Price price) const {
        if (tickSize <= 1) return price;
        return (price / tickSize) * tickSize;
    }

    Quantity RoundToLot(Quantity quantity) const {
        if (lotSize <= 1) return quantity;
        return (quantity / lotSize) * lotSize;
    }
};

enum class RejectionReason {
    None,
    InvalidPrice,
    InvalidQuantity,
    BelowMinQuantity,
    AboveMaxQuantity,
    BelowMinNotional, // 서버 부하 방지를 위한 최소 주문 금액
    DuplicateOrderId, // 실수로 같은 주문을 두 번 보냈을 떄 생긴다. 
    InvalidOrderType, // 차후 지원 안하는 거래가 나올 시에, 사용할 예정 
    EmptyBook
};


// 주문이 유효 여부에 대한 정보를 들고 있는다. 
struct OrderValidation {
    bool isValid = true;
    RejectionReason reason = RejectionReason::None;

    static OrderValidation Accept() {
        return OrderValidation{true, RejectionReason::None};
    }

    static OrderValidation Reject(RejectionReason reason) {
        return OrderValidation{false, reason};
    }
};