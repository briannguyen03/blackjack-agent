#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

float Q[22][12][2][2];

typedef struct {
    int player_sum;
    int dealer_upcard;
    int usable_ace;
} State;

#define MAX_CARDS 10
#define DECK_SIZE 52

int deck_index = 0;

typedef struct {
    char suit;
    char rank;
} Card;

typedef struct {
    int total;
    float chips;
    int hand_index;
    Card cards[MAX_CARDS];
} Hand;

time_t startTime, endTime;
double elapseTime;

void init_deck(Card deck[]) {
    const char suits[] = {'H', 'D', 'C', 'S'};
    const char ranks[] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'};
    int index = 0;

    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 13; r++) {
            deck[index].suit = suits[s];
            deck[index].rank = ranks[r];
            index++;
        }
    }
}

void shuffle_deck(Card deck[]) {
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

void ensure_deck_available(Card deck[], int amt) {
    if (deck_index + amt > DECK_SIZE) {
        init_deck(deck);
        shuffle_deck(deck);
        deck_index = 0;
    }
}

int card_value(Card card) {
    if (card.rank >= '2' && card.rank <= '9') return card.rank - '0';
    if (card.rank == 'T' || card.rank == 'J' || card.rank == 'Q' || card.rank == 'K') return 10;
    if (card.rank == 'A') return 11;
    return 0;
}

int calc_hand(Hand *hand) {
    int value = 0;
    int aces = 0;

    for (int i = 0; i < hand->hand_index; i++) {
        value += card_value(hand->cards[i]);
        if (hand->cards[i].rank == 'A') aces++;
    }

    while (value > 21 && aces > 0) {
        value -= 10;
        aces--;
    }

    hand->total = value;
    return value;
}

void print_hand(Hand* hand) {
    printf("[ ");
    for (int i = 0; i < hand->hand_index; i++) {
        printf("%c%c", hand->cards[i].rank, hand->cards[i].suit);
        if (i < hand->hand_index - 1) printf(", ");
    }
    printf(" ]\n");
}

void deal_hand(Hand* hand, Card deck[], int amt) {
    if ((hand->hand_index + amt) > MAX_CARDS) {
        printf("Sorry, max hand limit reached cannot deal more cards\n");
        return;
    } else if (deck_index + amt > DECK_SIZE) {
        printf("No more cards in deck — reshuffling.\n");
        init_deck(deck);
        shuffle_deck(deck);
        deck_index = 0;
    }

    for (int i = 0; i < amt; i++) {
        if (deck_index >= DECK_SIZE) {
            init_deck(deck);
            shuffle_deck(deck);
            deck_index = 0;
        }
        hand->cards[hand->hand_index] = deck[deck_index];
        deck_index++;
        hand->hand_index++;
    }
}

float betting_round(Hand* player) {
    float value = 0.0f;

    while (1) {
        printf("Calling all bets!\nYou have: $%.2f how much do you want to bet? (type a number from 1-%.2f)\n",
               player->chips, player->chips);

        if (scanf("%f", &value) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if (value <= 0.0f) {
            printf("Bet must be greater than 0. Try again.\n");
            continue;
        } else if (value > player->chips) {
            printf("Sorry you can't bet more than you have. Please try again!\n");
            continue;
        } else {
            player->chips -= value;
            printf("All bets are closed, beginning game...\n");
            break;
        }
    }

    return value;
}

int has_usable_ace(Hand* hand){
    int value = 0;
    int aces = 0;

    for (int i = 0; i < hand->hand_index; i++) {
        value += card_value(hand->cards[i]);
        if (hand->cards[i].rank == 'A') aces++;
    }

    while (value > 21 && aces > 0) {
        value -= 10;
        aces--;
    }

    return (aces > 0) ? 1:0;
}

void printQtable(FILE* fp){
    for(int i=0; i<22; i++)
        for(int j=0; j<12; j++)
            for(int k=0; k<2; k++)
                for(int l=0; l<2; l++)
                    fprintf(fp, "Q[%d][%d][%d][%d]: %f\n",i,j,k,l, Q[i][j][k][l]);
}

int dealer_upcard(Hand *dealer){
    int raw_val = card_value(dealer->cards[0]);
    int upcard;

    if (dealer->cards[0].rank == 'A') {
        upcard = 1;
    } else if (raw_val > 10) {
        upcard = 10;
    } else {
        upcard = raw_val;
    }
    
    return upcard;
}

void startTrain(){
    float alpha = 0.1618;
    float gamma = 0.899;
    float epsilon_start = 0.314;
    float epsilon_end = 0.02718;
    
    int episodes = 100000;
    float reward = 0.0;
    float epsilon_decay = (epsilon_start - epsilon_end) / episodes;

    int winc = 0;
    int loss = 0;
    int ties = 0;

    Card deckT[DECK_SIZE];
    Hand ai = {0, 100.0f, 0, {{0}}};
    Hand dealer = {0, 0.0f, 0, {{0}}};

    startTime = time(NULL);
   
    for (int episode = 1; episode<=episodes; episode++){
        float epsilon = epsilon_start - (episode * epsilon_decay);
        if (epsilon < epsilon_end) epsilon = epsilon_end;
        
        ai.hand_index = 0;
        dealer.hand_index = 0; 
        ai.total = 0; 
        dealer.total = 0;
        init_deck(deckT);
        shuffle_deck(deckT);
        deck_index = 0;

        deal_hand(&ai, deckT, 2);
        deal_hand(&dealer, deckT, 2);
        calc_hand(&ai);
        calc_hand(&dealer);
        
        while(1){
            State cur_s; 
            int ps = ai.total;
            if(ps<4) ps =4;
            if(ps>21) ps =21;
            cur_s.player_sum = ps;
            cur_s.dealer_upcard = dealer_upcard(&dealer);
            cur_s.usable_ace = has_usable_ace(&ai);

            if(ai.total > 21){
                reward = -1.0;
                break;
            }

            int action;
            float randval = (float)rand() / RAND_MAX;

            if(randval < epsilon){
                action = rand() % 2;
            }else {
                float q1 = Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][0];
                float q2 = Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][1];

                if(q1 > q2) {
                    action = 0;
                }else if (q2 > q1){
                    action = 1;
                } else { 
                    action = rand() %2;
                }
            }

            if (action  == 0){
                while(calc_hand(&dealer) < 17){
                    deal_hand(&dealer, deckT, 1);
                }

                if(dealer.total > 21 || ai.total > dealer.total){
                    reward = 1.0;
                }else if (dealer.total == ai.total){
                    reward = 0.0;
                }else{
                    reward = -1.0;
                }

                float old_q = Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][action];
                Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][action] = old_q + alpha * (reward - old_q);
                break;
            }else{
                deal_hand(&ai, deckT, 1);
                calc_hand(&ai);

                State next_s;
                int next_ps = ai.total;
                if(next_ps<4) next_ps=4;
                if(next_ps>21) next_ps=21;
                next_s.player_sum = next_ps;
                next_s.dealer_upcard = dealer_upcard(&dealer);
                next_s.usable_ace = has_usable_ace(&ai);

                if(ai.total > 21) {
                    reward = -1.0;

                    float old_q = Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][action];
                    Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][action] = old_q + alpha * (reward - old_q);
                    
                    break;
                }

                if (ai.total == 21) {
                    reward = 1.0;
                    float old_q = Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][action];
                    Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][action] =
                        old_q + alpha * (reward - old_q);
                    break;
                }

                reward = 0.0;
                
                float max_next_q = Q[next_s.player_sum][next_s.dealer_upcard][next_s.usable_ace][0];
                float q1_next = Q[next_s.player_sum][next_s.dealer_upcard][next_s.usable_ace][1];
                if (q1_next > max_next_q) max_next_q = q1_next;

                float old_q = Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][action];
                Q[cur_s.player_sum][cur_s.dealer_upcard][cur_s.usable_ace][action] += alpha * (reward + (gamma * max_next_q) - old_q);
        }
     }

        if(reward > 0) winc++;
        else if (reward < 0) loss++;
        else ties++;

        if (episode % 100000 == 0){
            float ratio = (float)winc / episode * 100;
            printf("Episode %d/%d | WIN rate: %.2f%% | Wins: %d, Losses %d, Ties %d\n", episode, episodes, ratio, winc, loss, ties);
        }
    }
    endTime = time(NULL);
    elapseTime = difftime(endTime, startTime);
    float final_win_rate = (float)winc / episodes * 100;
    printf("Training Complete! Elaspe Time: [%.2f]\n", elapseTime);
    printf("Final Stats - Win Rate: %.2f%% | Wins: %d, Losses: %d, Ties: %d\n",
           final_win_rate, winc, loss, ties);
}

void print_strategy_sample(){
    printf("\n=== Learned Strategy Samples ===\n");
    
    int test_states[][3] = {
        {16, 10, 0},
        {12, 2, 0},
        {18, 9, 1},
        {20, 6, 0}
    };
    
    for(int i = 0; i < 4; i++){
        int player = test_states[i][0];
        int dealer = test_states[i][1];
        int soft = test_states[i][2];
        
        float q_stand = Q[player][dealer][soft][0];
        float q_hit = Q[player][dealer][soft][1];
        
        printf("Player: %d%s | Dealer: %d | Q(Stand)=%.2f, Q(Hit)=%.2f | Best: %s\n",
               player, soft ? " (soft)" : "", dealer, q_stand, q_hit,
               q_hit > q_stand ? "HIT" : "STAND");
    }
}

void startGame(Card deck[], Hand* player, Hand* house) {
    init_deck(deck);
    shuffle_deck(deck);
    deck_index = 0;

    float bet = 0;
    int round = 1;
    char action = 0;

    player->hand_index = 0;
    house->hand_index = 0;
    player->total = 0;
    house->total = 0;

    bet = betting_round(player);

    deal_hand(player, deck, 2);
    deal_hand(house, deck, 2);

    printf("\nYou have betted $%.2f\n", bet);

    while (1) {
        printf("\n[--------------------------------------------------------]\n");
        printf("Round %d\n", round);
        printf("Your hand: \n");
        print_hand(player);
        printf("Total: %d\n", calc_hand(player));
        printf("\n");

        printf("Dealer hand: \n");
        if (house->hand_index > 0) {
            printf("[ %c%c, ?? ]\n", house->cards[0].rank, house->cards[0].suit);
        } else {
            printf("[ ?? ]\n");
        }

        if (player->total == 21 || house->total == 21) {
            printf("\nBlack Jack, %s won!\n", player->total == 21 ? "You" : "Dealer");
            if (player->total == 21) {
                player->chips = player->chips + (3 * bet);
                printf("Payout: %.2f\n", (3 * bet));
                bet = 0.0f;
            }
            break;
        } else if (player->total > 21 || house->total > 21) {
            printf("\nBust, %s lost!\n", player->total > 21 ? "You" : "Dealer");
            if (house->total > 21) {
                player->chips = player->chips + (2 * bet);
                printf("Payout: %.2f\n", player->chips);
                bet = 0;
            }
            break;
        }

        printf("\nWhat do you want to do?\n 1. Stand - 'S'\n 2. Hit - 'H'\n");
        if (scanf(" %c", &action) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf("Invalid input, assuming Hit.\n");
            action = 'h';
        } else {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            action = tolower(action);
        }

        if (action == 's') {
            printf("Choice: Stand\n");
            while (calc_hand(house) < 17) {
                deal_hand(house, deck, 1);
            }

            int playerTotal = calc_hand(player);
            int dealerTotal = calc_hand(house);

            printf("Your hand: ");
            print_hand(player);
            printf("Total: %d\n", playerTotal);

            printf("Dealer hand: ");
            print_hand(house);
            printf("Total: %d\n", dealerTotal);

            if (playerTotal > 21) {
                printf("You busted! Dealer wins.\n");
            } else if (dealerTotal > 21) {
                printf("Dealer busted! You win!\n");
                player->chips += 2 * bet;
            } else if (playerTotal > dealerTotal) {
                printf("You win!\n");
                player->chips += 2 * bet;
            } else if (playerTotal == dealerTotal) {
                printf("Push (tie)\n");
                player->chips += bet;
            } else {
                printf("Dealer wins.\n");
            }

            bet = 0.0f;
            break;

        } else {
            printf("Choice: Hit\n");
            deal_hand(player, deck, 1);
            calc_hand(player);
        }

        if (player->total == 21 || house->total == 21) {
            printf("[--------------------------------------------------------]\n");
            printf("\nBlack Jack, %s won!\n", player->total == 21 ? "You" : "Dealer");
            printf("Your hand: \n");
            print_hand(player);
            printf("Total: %d\n", calc_hand(player));
            if (player->total == 21) {
                player->chips = player->chips + (3 * bet);
                printf("Payout: %.2f\n", (3 * bet));
                bet = 0.0f;
            }
            printf("[--------------------------------------------------------]\n");
            break;
        }

        round++;
    }

    player->hand_index = 0;
    house->hand_index = 0;
    player->total = 0;
    house->total = 0;
}

int main() {
    FILE* fp = fopen("Qtablelog.txt","w");
    if(!fp){
        perror("Error opening log file");
    }
    printf("Initializing Training\n");
    for(int i=0; i<22; i++)
        for(int j=0; j<12; j++)
            for(int k=0; k<2; k++)
                for(int l=0; l<2; l++)
                    Q[i][j][k][l] = 0;
    
    srand((unsigned)time(NULL));
    printf("Welcome to Black Jack!\n");

    Card deck1[DECK_SIZE];
    Hand player = {0, 100.0f, 0, {{0}}};
    Hand dealer = {0, 0.0f, 0, {{0}}};

    char input = 0;

    init_deck(deck1);

    do {
        printf("You currently have: $%.2f\n", player.chips);
        printf("Select the mode you would like to enter\n E. Player Mode\n A. Ai mode\n");

        if (scanf(" %c", &input) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input, exiting.\n");
            break;
        }
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);

        if (tolower(input) == 'e') {
            startGame(deck1, &player, &dealer);
        }else if (tolower(input) == 'a'){
            printf("Training!\n");
            startTrain();
            printQtable(fp);
            print_strategy_sample();
        }else{
            printf("Goodbye!\n");
            break;
        }

        if (player.chips <= 0) {
            printf("You're out of chips! Game over.\n");
            break;
        }

    } while (1);

    fclose(fp);
    return 0;
}