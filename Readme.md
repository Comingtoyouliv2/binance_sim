# Orderbook Simulator — 학습 가이드

C++20 으로 작성된 Limit Order Book 매칭 엔진 학습 노트입니다.
원본 레포: [SLMolenaar/orderbook-simulator-cpp](https://github.com/SLMolenaar/orderbook-simulator-cpp)
자세한 사항은 fileDescription.txt 를 확인하세요

---

## 파일 구조

```
Types.h → Ordertype.h → Constants.h → LevelInfo.h
                    ↓
         Order.h → Trade.h → OrderModify.h
                    ↓
              MarketDataFeed.h · ExchangeRules.h
                    ↓
               Orderbook.h ★
                    ↓
         tests.cpp · LiveMarketData.cpp
```

---

## Step 1 — 기반 타입 레이어

**1. `Types.h`** — 프로젝트 전체 공통 타입 정의
- `Price(int32_t)`, `Quantity(uint32_t)`, `OrderId(uint64_t)`
- `OrderPointers = vector<OrderPointer>` → FIFO 로 Price-Time Priority 관리

**2. `Ordertype.h`** — `enum class` 로 `OrderType` / `Side` 정의
- `enum class` 사용 이유: 타입 혼동을 컴파일 타임에 방지

**3. `Constants.h`** — `InvalidPrice = numeric_limits<Price>::min()`
- `numeric_limits` 사용 이유: 나중에 `int64_t` 로 타입 변경시 자동 조정

**4. `LevelInfo.h`** — 호가창 스냅샷 구조체
- `bids_{bids}` : 초기화 리스트로 멤버 바로 세팅

**5. `Trade.h`** — 체결 결과 기록
- 체결은 항상 쌍 → `Trade = { bidTrade, askTrade }`
- 주문 넣는 시점이 아닌 **체결 순간**에 생성
- `Trades = vector<Trade>` → 부분 체결 여러 번 가능

---

## Step 2 — 엔티티 레이어

**6. `Order.h`** — 작성 중...