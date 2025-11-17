# blackjack-agent

A blackjack agent that uses **Q-learning** to learn and play blackjack.

---

## Installation

1. Clone the repository:

```bash
git clone https://github.com/yourusername/blackjack-agent.git
```

2. Compile the program:

```bash
gcc blackjack.c -o blackjack
```

---

## Usage

The program has **two modes**:

* **Regular game:** Enter `E` or `e` to play blackjack manually.
* **Agent training:** Enter `A` or `a` to train the Q-learning agent.

Before training, check out the `startTrain()` function in `blackjack.c`. Important **hyperparameters** for Q-learning:

```c
float alpha         // learning rate
float gamma         // discount factor for future rewards
float epsilon_start // starting exploration rate
float epsilon_end   // ending exploration rate
int episodes        // number of training games (default 10M)
```

**Bellman Equation used for Q-learning:**

```
Q(s,a) = R(s,a) + γ max_a Q(s',a)
```

Where:

* `Q(s, a)` = Q-value for a state-action pair
* `R(s, a)` = immediate reward for taking action `a` in state `s`
* `γ` (gamma) = discount factor for future rewards
* `max_a Q(s',a)` = max Q-value for the next state `s'` over all possible actions

> Reference: [GeeksforGeeks Q-learning](https://www.geeksforgeeks.org/machine-learning/q-learning-in-python/)

**Training Output:**

* Status updates every 100k games (would change this based on how many episodes you are doing)
* Final stats example:

```
Final Stats - Win Rate: 40.72% | Wins: 4,071,563 | Losses: 5,209,174 | Ties: 719,263
```

* After training, a `Qtablelog.txt` file containing the **Q-table** is generated for persistent storage or verification.

**Tip:** Tweaking hyperparameters can improve win rate, but it generally doesn’t exceed ~45% (or at least I haven't been able to do so).

