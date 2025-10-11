
/* Move Ordering Hierarchy
1. TT move
2. Captures (scored by MVV-LVA and SEE >= 0)
3. Killer Moves (cached)
4. Quiet Moves (use history heuristic)
5. Bad Captures (SEE < 0)

* Will add SEE later
*/

struct MoveSelector {
    // Keep track of phases
    // Phase 0: Return TT move (no TT right now, so we'll skip)
    // Phase 1: Return captures (run move generator with capture gen mode)
    //  - Evaluate all captures with MVV - LVA, return moves in sorted order
    //  - Later - only evaluate moves with SEE >= 0
    // Phase 2: Killer Moves - moves that caused a cutoff earlier (cached)
    // Phase 3: Quiet Moves (run move generator with quiet gen mode and sort by history/continuation/countermove)
    // Phase 4: Remaining moves (bad captures with SEE < 0)
    //  - No SEE right now so this phase will be skipped

    // Update bench to add tests for phased move generation

};