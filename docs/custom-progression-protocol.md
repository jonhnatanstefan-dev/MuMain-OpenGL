# Custom progression status (F3/60)

This client keeps the stock OpenMU Season 6 protocol intact and adds one
**optional** server-to-client packet for Reset/Grand Reset/VIP integration.

## Packet contract

- Main code: `0xF3`
- Subcode: `0x60`
- Direction: game server -> client
- Header: C1 / `PBMSG_HEADER`
- Packed payload:

| Field | Type | Meaning |
| --- | --- | --- |
| SubCode | byte | `0x60` |
| Resets | uint32 | Current character reset count |
| GrandResets | uint32 | Independent Grand Reset count |
| VipLevel | byte | VIP tier/level; 0 means no VIP |
| VipRemainingSeconds | uint32 | Remaining VIP time |

The server should send the packet after world entry and after any Reset,
Grand Reset, VIP purchase/expiration or administrative change.

## 18 - Reset

Stock OpenMU already exposes `/reset` and `/resetinfo` and sends the reset
counter in the extended join packet. The client therefore does not duplicate
level/cost/item/point rules. Clicking the small **i** beside the level/reset
line sends `/resetinfo`; OpenMU remains authoritative for required level,
zen, item quantity, point reward, reset limit and stat maintenance/reset policy.

## 19 - Grand Reset

`GrandResets` is independent from `Resets`. Requirements, rewards, limits
and any conversion/consumption of normal resets must be calculated and persisted
by the server, then reflected through F3/60.

## 20 - VIP

`VipLevel` is a tier number rather than a boolean, allowing configurable
Free/Bronze/Silver/Gold (or any other) tiers without another client protocol
change. Benefits stay server-side. `VipRemainingSeconds` reserves expiration
status for the UI without changing the packet later.
