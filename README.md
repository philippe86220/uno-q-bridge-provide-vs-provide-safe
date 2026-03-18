## Objective

This example demonstrates how `Bridge.provide_safe()` changes the execution model of a callback.

Unlike `Bridge.provide()`, the callback is not executed in a separate background thread.  
It runs in the same execution context as `loop()`, but its execution is deferred.

The goal is to observe when the callback is actually executed.

---

## Test code (MCU)

```cpp
#include <Arduino.h>
#include <Arduino_RouterBridge.h>

void printSlow(const char *s) {
  for (int i = 0; s[i] != '\0'; i++) {
    Monitor.print(s[i]);
    delay(20);
  }
  Monitor.println();
}

void callbackTest(int x) {
  Monitor.println(">>> CALLBACK EXECUTED <<<");
}

void setup() {
  Bridge.begin();
  Monitor.begin();

  Bridge.provide_safe("test", callbackTest);
  // Replace with Bridge.provide("test", callbackTest) to compare behavior
}

void loop() {
  Monitor.println("[LOOP START]");

  printSlow("Forum");

  delay(100);

  Monitor.println("[LOOP END]");
}
```

---

## Test code (MPU)

```python
from arduino.app_utils import App, Bridge
import time

def loop():
    Bridge.call("test", 0)
    time.sleep(0.05)

App.run(user_loop=loop)
```

---

## Observed behavior

In this experiment, the output consistently shows:

```
[LOOP START]
Forum
[LOOP END]
>>> CALLBACK EXECUTED <<<
```

This indicates that:

- the callback is NOT executed during user code execution (e.g. inside `printSlow()`)
- the callback is NOT executed during `delay()`
- the callback is executed after the user code in `loop()` has completed

So the execution model is:

```
loop() execution
   ↓
loop() ends
   ↓
safe callback is executed
   ↓
next loop iteration
```

---

## Interpretation

`Bridge.provide_safe()` does not rely on concurrent execution.

Instead:

- the callback is executed in the same execution context as `loop()`
- its execution is deferred and handled by the framework
- it is not triggered by scheduler preemption

In this experiment, this results in the callback being executed after the complete execution of `loop()`, rather than during user code execution.

This behavior is observed consistently, but should be understood as a consequence of the framework’s execution model, not as a guarantee of a specific instruction-level insertion point.

---

## Comparison with Bridge.provide()

If you replace:

```
Bridge.provide_safe("test", callbackTest);
```

with:

```
Bridge.provide("test", callbackTest);
```

then the callback runs in a separate thread.

In this case:

- the callback can interrupt `loop()`
- it can execute during `delay()`
- it can interleave with functions like `printSlow()`

Example of possible output:

```
[LOOP START]
Forum
>>> CALLBACK EXECUTED <<<
>>> CALLBACK EXECUTED <<<
[LOOP END]
[LOOP START]
For>>> CALLBACK EXECUTED <<<
um
>>> CALLBACK EXECUTED <<<
>>> CALLBACK EXECUTED <<<
[LOOP END]
[LOOP START]
Fo>>> CALLBACK EXECUTED <<<
rum>>> CALLBACK EXECUTED <<<
```

---

## Key difference

### Bridge.provide()

- callback runs in a separate execution context (RPC thread)
- concurrent with `loop()`
- can interrupt user code at any time

### Bridge.provide_safe()

- callback runs in the same execution context as `loop()`
- execution is deferred and managed by the framework
- not executed concurrently with user code
- not triggered by scheduler preemption

---

## Conclusion

`Bridge.provide_safe()` does not simply protect the callback.

It changes its execution model:

- from **concurrent execution** (`provide`)
- to **deferred execution in the loop context** (`provide_safe`)

This ensures that:

- user code is not interrupted by concurrent execution
- no interleaving occurs with a separate thread
- execution is safe without explicit synchronization in this context

In this experiment, the callback is observed to run after the complete execution of `loop()`, but this should be understood as a consequence of the execution model rather than a guaranteed insertion point.

---

## Author

Philippe86220
