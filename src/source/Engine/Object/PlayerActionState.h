#pragma once

#include "Core/Globals/_enum.h"

namespace Engine::Object
{
    // True only for actual attack/skill animations.
    //
    // Do not use one broad PLAYER_ATTACK_FIST..PLAYER_RIDE_SKILL range here:
    // that numeric span also contains Dark Horse/Fenrir stand, run, walk and
    // damage animations. Treating a mounted idle pose as an attack makes the
    // MU Helper believe a swing is permanently in progress and it never fires.
    inline bool IsAttackAction(int currentAction)
    {
        if (currentAction >= PLAYER_ATTACK_FIST && currentAction < PLAYER_ATTACK_END)
            return true;

        if (currentAction >= PLAYER_ATTACK_STRIKE && currentAction <= PLAYER_ATTACK_DARKHORSE)
            return true;

        if (currentAction >= PLAYER_FENRIR_ATTACK && currentAction <= PLAYER_FENRIR_SKILL_ONE_LEFT)
            return true;

        if (currentAction >= PLAYER_ATTACK_BOW_UP && currentAction <= PLAYER_HIGH_SHOCK)
            return true;

        if (currentAction == PLAYER_ATTACK_TWO_HAND_SWORD_TWO)
            return true;

        if (currentAction >= PLAYER_SKILL_HAND1 && currentAction <= PLAYER_RIDE_SKILL)
            return true;

        if (currentAction == PLAYER_RAGE_UNI_ATTACK ||
            currentAction == PLAYER_RAGE_UNI_ATTACK_ONE_RIGHT ||
            currentAction == PLAYER_RAGE_FENRIR_ATTACK_RIGHT)
            return true;

        return false;
    }

    // True while a player is sitting or holding a pose (PLAYER_SIT1 ..
    // PLAYER_POSE_FEMALE1). Used to keep these animations from being reset to the
    // stand pose when equipment or class changes.
    inline bool IsSitOrPoseAction(int currentAction)
    {
        return currentAction >= PLAYER_SIT1 && currentAction <= PLAYER_POSE_FEMALE1;
    }
}
