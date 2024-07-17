// struct HailCapabilityTicker
// {
//     CapabilityTickSortable Sortable;

//     HailCapability Capability = nullptr;

//     void Tick(float DeltaTime) const
//     {
//         if (!Capability.bDidLateSetup)
//         {
//             Capability.bDidLateSetup = true;
//             Capability.LateSetup();
//         }

//         bool bIsBlocked = Capability.IsBlocked();
//         bool bShouldBeActive = Capability.ShouldActivate() && !bIsBlocked;
        
//         if (!Capability.bActive && bShouldBeActive)
//         {
//             Capability.bActive = true;
//             Capability.ActiveDuration = 0.0;
//             Capability.InactiveDuration = 0.0;
//             Capability.OnActivated();
//         }
//         else if (Capability.bActive && (Capability.ShouldDeactivate() || bIsBlocked))
//         {
//             Capability.bActive = false;
//             Capability.ActiveDuration = 0.0;
//             Capability.InactiveDuration = 0.0;
//             Capability.OnDeactivated();
//         }

//         if (Capability.bActive)
//         {
//             Capability.TickActive(DeltaTime * Capability.GetTimeDilation());
//             Capability.ActiveDuration += DeltaTime * Capability.GetTimeDilation();
//         }
//         else
//         {
//             Capability.TickInactive(DeltaTime * Capability.GetTimeDilation());
//             Capability.InactiveDuration += DeltaTime * Capability.GetTimeDilation();
//         }
//     }

//     void SetSorting(FCapabilityTickSortable NewSorting)
//     {
//         Sortable = NewSorting;
//     }

//     FCapabilityTickSortable GetSorting() const
//     {
//         return Sortable;
//     }
// };
