#pragma once

enum class OrderType {
    GoodTillCancel, // Active until completely filled 
    ImmediateOrCancel, // Fill for as far as possible and kill immed
    Market, //Fill at any price
    GoodForDay, // All of these are cacelled at a specific time every day
    FillOrKill // Fill fully or kill
};

enum class Side {
    Buy,
    Sell
};