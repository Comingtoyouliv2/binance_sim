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

            Price orderPrice = order->GetPrice();

            /**
             * AddOrder() 에 대한 exception 처리이다. 다시 확인할 필요가 있음 
             */
            bool isConvertedMarketOrder = 
                (orderPrice == std::numeric_limits<Price>::max() ||
                 orderPrice == std::numeric_limits<Price>::min());

            // 불가능한 가격이라면, Reject 하는 경우들에 대한 처리 
            if (!isConvertedMarketOrder) {
                if (!exchangeRules_.IsValidPrice(orderPrice)) {
                    return OrderValidation::Reject(RejectionReason::InvalidPrice);
                }

                if(!exchangeRules_.IsValidQuantity(order->GetRemainingQuantity())) {
                    if (order->GetRemainingQuantity() < exchangeRules_.minQuantity) {
                        return OrderValidation::Reject(RejectionReason::BelowMinQuantity);
                    } else if (order->GetRemainingQuantity() > exchangeRules_.maxQuantity) {
                        return OrderValidation::Reject(RejectionReason::AboveMaxQuantity);
                    } else {
                        return OrderValidation::Reject(RejectionReason::InvalidQuantity);
                    }
                }
                
                if (!isConvertedMarketOrder) {
                    if (!exchangeRules_.IsValidNotional(orderPrice, order->GetRemainingQuantity())) {
                        return OrderValidation::Reject(RejectionReason::BelowMinNotional);
                    }
                }

                return OrderValidation::Accept();
            }
        }


        void CheckAndResetDay() {}
        void CancelGoodForDayOrder(){}
        std::vector<std::pair<OrderPointer, Quantity>> CollectMatchesForFillOrKill() {}
        Trades ExecuteMatchesForFillorKill() {}
        Trades MatchFillOrKill(OrderPointer order) {}
        Trades MatchOrders() {}
        void ProcessNewOrder() {}
        void ProcessModify() {}
        void ProcessTrade() {}
        void ProcessSnapshot() {}

    public:
        //객체 생성할때 lastDayReset_ 을 현재시간으로 초기화한다. 
        Orderbook(): lastDayReset_(std::chrono::system_clock::now()) {}

        void SetExchangeRules() {}
        const ExchangeRules &GetExchangeRules() const {return exchangeRules_;}

        void SetDayResetTime() {}

        /**
         * @brief 주문 종류별로 주문을 처리하는 함수 
         * 
         * @param 
         * @param 
         * @return 
         */
        Trades AddOrder(OrderPointer order) {
            if (order->GetOrderType() == OrderType::Market) {
                if (order->GetSide() == Side::Buy && !asks_.empty()) {
                    order->ToGoodTillCancel(std::numeric_limits<Price>::max());
                } else if (order->GetSide() == Side::Sell && !bids_.empty()) {
                    order->ToGoodTillCancel(std::numeric_limits<Price>::min());
                } else {
                    // 시장가 주문이지만, 반대편 호가창이 비어있는 경우, 주문을 거절한다. 
                    throw std::logic_error("Market order cannot be added when the opposite side of the book is empty.");
                }
            }

            auto validation = ValidateOrder(order);
            if (!validation.isValid) return {};

            if (order->GetOrderType() == OrderType::FillOrKill) {
                return MatchFillOrKill(order);
            }

            OrderPointers* ordersPtr;

            if (order->GetSide() == Side::Buy) {
                ordersPtr = &bids_[order->GetPrice()];
            } else {
                ordersPtr = &asks_[order->GetPrice()];
            }
            
            // 주문번호로 빠르게 찾기 위함, bids/asks 는 가격으로 저장해 특정 주문을 찾기 어렵다. 
            ordersPtr->push_back(order);
            orders_.insert({order->GetOrderId(), OrderEntry{order, ordersPtr->size() - 1}});

            // IOC = Immediate or Cancel (가격 조건 지키면서, 지금 가능한만큼만 체결)
            // Market Order 와는 다른점; 가격보다는 바로 살 수 있기를 원함
            const bool isIoc = (order->GetOrderType() == OrderType::ImmediateOrCancel);
            // std::optional<T> 는 값이 있을수도 있고, 없을 수도 있다고 알려준다. 
            return MatchOrders(isIoc ? order->GetOrderId() : std::optional<OrderId>{});
        }


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