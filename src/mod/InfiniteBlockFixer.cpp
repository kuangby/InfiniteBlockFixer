#include "mod/InfiniteBlockFixer.h"

#include "ll/api/memory/Hook.h"
#include "ll/api/mod/RegisterHelper.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockLegacy.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/actor/MovingBlockActor.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include "mc/world/level/block/registry/BlockTypeRegistry.h"


namespace infiniteBlockFixer {

InfiniteBlockFixer& InfiniteBlockFixer::getInstance() {
    static InfiniteBlockFixer instance;
    return instance;
}

LL_TYPE_INSTANCE_HOOK(
    InfiniteBlockFixerHook,
    ll::memory::HookPriority::Normal,
    MovingBlockActor,
    &MovingBlockActor::$tick,
    void,
    BlockSource& region
) {
    BlockActor::tick(region);
    // if ((static_cast<uint64>(mWrappedBlock->mLegacyBlock->mProperties) &
    // static_cast<uint64>(BlockProperty::Immovable))
    //     || (static_cast<uint64>(mWrappedExtraBlock->mLegacyBlock->mProperties)
    //         & static_cast<uint64>(BlockProperty::Immovable))) {
    //     auto const& airBlock = BlockTypeRegistry::getDefaultBlockState("minecraft:air");
    //     mWrappedBlock        = &airBlock;
    //     mWrappedExtraBlock   = &airBlock;
    // } // fix position
    mCollisionShape->min = Vec3(mPosition->x, mPosition->y, mPosition->z);
    mCollisionShape->max = Vec3(mPosition->x + 1, mPosition->y + 1, mPosition->z + 1);
    if ((!mPistonBlockPos->x && mPistonBlockPos->y == -1 && !mPistonBlockPos->z) // 保留原版特性
        || !region.getBlock(mPistonBlockPos).getTypeName().ends_with("piston")) {
        if (region.getBlock(mPosition).getTypeName() == "minecraft:moving_block") { // fix position
            region.setBlock(mPosition, *mWrappedBlock, 3, mWrappedBlockActor, 0, 0);
            region.setExtraBlock(mPosition, *mWrappedExtraBlock, 3);
            mWrappedBlock->mLegacyBlock->movedByPiston(region, mPosition);
        }
    }
}

bool mutex = false;

LL_TYPE_INSTANCE_HOOK(
    InfiniteBlockFixerHook2,
    ll::memory::HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_moveCollidedEntities,
    void,
    BlockSource& region
) {
    mutex = true;
    origin(region);
    mutex = false;
}

LL_TYPE_INSTANCE_HOOK(
    InfiniteBlockFixerHook3,
    ll::memory::HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_spawnMovingBlock,
    void,
    BlockSource&    region,
    BlockPos const& blockpos
) {
    if (!mutex) origin(region, blockpos);
}

bool InfiniteBlockFixer::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool InfiniteBlockFixer::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    InfiniteBlockFixerHook::hook();
    InfiniteBlockFixerHook2::hook();
    InfiniteBlockFixerHook3::hook();
    return true;
}

bool InfiniteBlockFixer::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace infiniteBlockFixer

LL_REGISTER_MOD(infiniteBlockFixer::InfiniteBlockFixer, infiniteBlockFixer::InfiniteBlockFixer::getInstance());
