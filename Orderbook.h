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

        /** 
         * @brief map<Key, Value, Comparator> 로 Price priority 를 구현
        */
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
         * @brief 주어진 가격과 방향(side)가 즉시 체결이 가능한지 확인한다
         * 
         * 현재 오더북의 최우선 가격(best bid/ask)와 비교하여,
         * 주문이 체결 가능한 상태인지 판단한다. 
         * 
         * @param price 주문 가격
         * @param side 매수/매도 여부
         * @return true 즉시 체결 가능
         * @return false 체결 불가능
         */
        bool CanMatch(Price price, Side side) const {
            if(side == Side::Buy) {
                if(asks_.empty()) return false;
                // 매도 호가창에서 가장 낮은 가격을 bestAsk 에 저장 
                const auto &[bestAsk, _] = *asks_.begin();
                return price >= bestAsk;
            } else {
                if (bids_.empty()) return false; 
                const auto &[bestBid, _] = *bids_.begin();
                return price <= bestBid;
            }
        }


        /**
         * @brief 
         * 
         * @param 
         * @param 
         * @return 
         */
        OrderValidation ValidateOrder(OrderPointer order) const {
            if (orders_.contains(order->GetOrderId())) {
                return OrderValidation::Reject(RejectionReason::DuplicateOrderId);
            }
        }


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