#pragma once

#include <map>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <vector>
#include <optional>
#include <iostream>
#include <sstream>

#include "Order.h"
#include "OrderModify.h"
#include "Trade.h"
#include "LevelInfo.h"
#include "types.h"
#include "OrderType.h"
#include "Constants.h"
#include "MarketDataFeed.h"
#include "ExchangeRules.h"


class Orderbook {
    private:
        struct OrderEntry {
            OrderPointer order_{nullptr};
            std::size_t location_; 
        };

        std::map<Price, OrderPointers, std::greater<Price>> bids_;
        std::map<Price, OrderPointers, std::less<Price>> asks_;
        std::unordered_map<OrderId, OrderEntry> orders_;

        std::chrono::system_clock::time_point lastDayReset_;
        std::chrono::hours dayResetHour_{15};
        int dayResetMinute_{59};

        MarketDataStats stats_;
        uint64_t lastSequenceNumber_{0};
        bool isInitialized_{false};
        
        ExchangeRules exchangeRules_;

        /**
         * @brief 매수 가격이 체결 가능하지 확인하는 함수
         * 
         * @param 
         * @return 
         */
        bool CanMatch(Price price, Side side) const {
            if(side == Side::Buy) {
                
            }
        }
        OrderValidation ValidateOrder() {}
        void CheckAndResetDay() {}
        void CancelGoodForDayOrder(){}
        std::vector<std::pair<OrderPointer, Quantity>> CollectMatchesForFillOrKill() {}
        Trades ExecuteMatchesForFillorKill() {}
        Trades MatchOrders() {}
        void ProcessNewOrder() {}
        void ProcessModify() {}
        void ProcessTrade() {}
        void ProcessSnapshot() {}

    public:
        Orderbook() {}

        void SetExchangeRules() {}
        const ExchangeRules &GetExchangeRules() const {return exchangeRules_;}

        void SetDayResetTime() {}
        Trades AddOrder() {}
        void CancelOrder() {}
        Trades MatchOrder() {}

        std::size_t Size() const {return orders_.size();}

        OrderbookLevelInfos GetOrderInfos() const {}
        bool ProcessMarketData() {}

        size_t ProcessMarketDataBatch() {}
        const MarketDataStats &GetMarketDataStats() const {}
        void ResetMarketDataStats() {}
        bool IsInitialized() const {return isInitialized_;}
        uint64_t GetLastSequenceNumber() const {return lastSequenceNumber_;}
        
}