## Memory Note (Stack/Heap Collision)
If you see freezes or random behavior and suspect a stack/heap collision, you can reduce stack usage by adjusting two constants inside the LcdMenu library:

1. **Reduce per-item draw buffer**
   - File: `C:\Users\Iheb\Documents\Arduino\libraries\LcdMenu\src\MenuItem.h`
   - Change: `ITEM_DRAW_BUFFER_SIZE 25 -> 17`
   - Effect: Shrinks temporary buffers used when rendering widget values (stack savings).

2. **Reduce printf conversion buffers**
   - File: `C:\Users\Iheb\Documents\Arduino\libraries\LcdMenu\src\utils\printf.c`
   - Change:
     - `PRINTF_NTOA_BUFFER_SIZE 32 -> 16`
     - `PRINTF_FTOA_BUFFER_SIZE 32 -> 16`
   - Effect: Smaller formatting buffers reduce stack pressure.

**Tradeoff:** Very long numbers or strings may be truncated. This is usually safe for 16x2 LCDs.
