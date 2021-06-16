#include <cs50.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void add_pair(int winner, int loser);
void sort_pairs(void);
int cmpstrength(const void *a, const void *b);
void lock_pairs(void);
bool cycle_check(int winner, int loser);
void cycle_check_recur(int winner, int loser, bool *makes_cycle);
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);
        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    //check if name is a valid candidate
    for (int k = 0; k < candidate_count; k++)  //candidate_count also the length of ranks
    {
        if (strcmp(name, candidates[k]) == 0)  //if the submitted name matches a valid candidate
        {
            ranks[rank] = k;  //set the rank to that candidate's index in candidates
            return true;
        }
    }
    return false;
}

// Update preferences given one voter's ranks - references initialized with 0s since defined in global scope
void record_preferences(int ranks[])
{
    for (int i = 0; i < candidate_count - 1; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    for (int i = 0; i < candidate_count - 1; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            if (preferences[i][j] > preferences[j][i])
            {
                add_pair(i, j);
                pair_count++;
            }
            else if (preferences[i][j] < preferences[j][i])
            {
                add_pair(j, i);
                pair_count++;
            }
        }
    }
    return;
}

//add a pair
void add_pair(int winner, int loser)
{
    pair new_pair;
    new_pair.winner = winner;
    new_pair.loser = loser;
    pairs[pair_count] = new_pair;
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    qsort(pairs, pair_count, sizeof(pair), cmpstrength);
    return;
}

//define comparison function to be used by qsort in sort_pairs
int cmpstrength(const void *a, const void *b)
{
    pair pair1 = *(pair *)a;
    pair pair2 = *(pair *)b;
    int pair1_strength = preferences[pair1.winner][pair1.loser];
    int pair2_strength = preferences[pair2.winner][pair2.loser];
    if (pair1_strength > pair2_strength)
    {
        return -1;
    }
    else if (pair1_strength < pair2_strength) // pair2 is stronger
    {
        return 1;
    }
    else  //they are equally strong
    {
        return 0;
    }
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    for (int i = 0; i < pair_count; i++)
    {
        pair cur_pair = pairs[i];
        if (!cycle_check(cur_pair.winner, cur_pair.loser))
        {
            locked[cur_pair.winner][cur_pair.loser] = true;
        }
    }
    return;
}

//check for cycles
bool cycle_check(int winner, int loser)
{
    bool makes_cycle = false;
    locked[winner][loser] = true; //temporarily lock this pair for the cycle check
    cycle_check_recur(winner, loser, &makes_cycle);
    locked[winner][loser] = false; //unlock the temporary lock
    return makes_cycle;
}

void cycle_check_recur(int winner, int loser, bool *makes_cycle)
{
    if (winner == loser)  //the recursion has reached back to the original winner, creating a cycle
    {
        *makes_cycle = true;
    }
    else  //check all locked pairs and recurse anywhere the current loser is a winner
    {
        for (int j = 0; j < pair_count; j++)
        {
            pair cur = pairs[j];
            if (cur.winner == loser)
            {
                if (locked[cur.winner][cur.loser])
                {
                    cycle_check_recur(winner, cur.loser, makes_cycle);
                }
            }
        }
    }
}

// Print the winner of the election
void print_winner(void)
{
    for (int j = 0; j < candidate_count; j++)
    {
        bool winner = true;
        for (int i = 0; i < candidate_count; i++)
        {
            if (locked[i][j]) // this candidate has been beaten by someone
            {
                winner = false;
            }
        }
        if (winner)
        {
            printf("%s\n", candidates[j]);
            return;
        }
    }
    printf("No winner - something is wrong\n");
    return;
}