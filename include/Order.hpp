enum OrderType { Buy, Sell, NoOrder, NoInfo, LowScoreLong, LowScoreShort, LowTpSlRatioLong, LowTpSlRatioShort};

class Order{

    public:
        OrderType orderType;  
        double entryPrice;
        double stopLossPrice;
        double tpPrice;

        double trustScore;

        Order(OrderType type = NoOrder, double entry = 0.0, double stopLoss = 0.0, double tp = 0.0)
        : orderType(type), entryPrice(entry), stopLossPrice(stopLoss), tpPrice(tp) {}

};