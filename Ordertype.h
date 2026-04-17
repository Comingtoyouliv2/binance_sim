#pragma once

enum class OrderType {
    GoodTillCancel, // 체결되기 전까지 살아있는 주문 (Limit order)
    ImmediateOrCancel, // 지금 당장 체결 가능한만큼만 체결, 남는 물량은 즉시 취소
    Market, // 현재 가격으로 체결 
    GoodForDay, // 하루만 유효한 주문, 장 마감시 자동 취소
    FillOrKill // 전부 체결 안되면 안하는 주문 
};

enum class Side {
    Buy,
    Sell
};