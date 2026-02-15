#pragma once
#include <map>
#include <vector>

#include "Inventory.h"

namespace fresh
{
namespace rpg
{

/**
 * @brief Trade offer (buy or sell)
 */
struct TradeOffer {
    ResourceType resource;
    float pricePerUnit;
    float availableQuantity;
    bool canBuy;  // Can player buy from this offer
    bool canSell; // Can player sell to this offer
};

/**
 * @brief Trading system for resource exchange
 */
class TradingSystem
{
public:
    TradingSystem();

    // Trade offer management
    void addOffer(const TradeOffer& offer);
    void removeOffer(ResourceType resource);
    const TradeOffer* getOffer(ResourceType resource) const;
    std::vector<TradeOffer> getAllOffers() const;

    // Trading operations
    bool buyResource(ResourceType resource, float quantity, Inventory& playerInventory,
                     float& playerCredits);
    bool sellResource(ResourceType resource, float quantity, Inventory& playerInventory,
                      float& playerCredits);

    // Price calculations
    float calculateBuyPrice(ResourceType resource, float quantity) const;
    float calculateSellPrice(ResourceType resource, float quantity) const;

    // Update prices based on supply/demand (optional feature)
    void updatePrices(float deltaTime);

private:
    std::map<ResourceType, TradeOffer> offers;

    void initializeDefaultOffers();
};

} // namespace rpg
} // namespace fresh
