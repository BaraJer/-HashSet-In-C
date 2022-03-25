
#include <stdlib.h>
#include "Node.h"
#include "HashSet.h"
#define ERR -1
#define ZERO 0
#define SUCCESS 1

/**
 * frees the memory of a list of nodes
 * @param hash_set
 */
void nodes_list_free (HashSet *hash_set)
{
  for (unsigned int i = 0; i < hash_set->capacity; ++i)
    {
      node_free (&(hash_set->NodesList[i]));
    }
  free (hash_set->NodesList);
  hash_set->NodesList=NULL;
}
/**
 * allocates memory for al list of nodes
 * @param value_cpy
 * @param value_cmp
 * @param value_free
 * @param new_size
 * @return
 */
Node **nodes_list_alloc (NodeElemCpy value_cpy, NodeElemCmp value_cmp,
                         NodeElemFree value_free, unsigned int new_size)
{
  Node **nodes_list = realloc (NULL, new_size * sizeof (Node *));
  for (unsigned int i = 0; i < new_size; ++i)
    {
      nodes_list[i] = node_alloc (value_cpy, value_cmp, value_free);
    }
  return nodes_list;

}
/**
 * Allocates dynamically new hash set.
 * @param hash_func a function which "hashes" keys.
 * @param value_cpy a function which copies Values.
 * @param value_cmp a function which compares Values.
 * @param value_free a function which frees Values.
 * @return pointer to dynamically allocated HashSet.
 * @if_fail return NULL or if one of the function pointers is null.
 */

HashSet *hash_set_alloc (
    HashFunc hash_func, HashSetValueCpy value_cpy,
    HashSetValueCmp value_cmp, HashSetValueFree value_free)
{
  if (!hash_func || !value_cpy || !value_cmp || !value_free)
    {
      return NULL;
    }
  Node **nodes_list = nodes_list_alloc (value_cpy, value_cmp, value_free,
                                        HASH_SET_INITIAL_CAP);
  HashSet *new_hash_set = malloc (sizeof (HashSet));
  new_hash_set->NodesList = nodes_list;
  new_hash_set->capacity = HASH_SET_INITIAL_CAP;
  new_hash_set->size = 0;
  new_hash_set->hash_func = hash_func;
  new_hash_set->value_cpy = value_cpy;
  new_hash_set->value_cmp = value_cmp;
  new_hash_set->value_free = value_free;

  return new_hash_set;
}

/**
 * Frees the hash set and all its elements.
 * @param p_hash_set pointer to pointer to a hash_set.
 */
void hash_set_free (HashSet **p_hash_set)
{
  if (!p_hash_set)
    {
      return;
    }
  if (!(*p_hash_set))
    {
      return;
    }
  for (size_t i = 0; i < (*p_hash_set)->capacity; i++)
    {
      if ((*p_hash_set)->NodesList[i])
        {
          node_free (&(*p_hash_set)->NodesList[i]);
        }
      else
        {
          return;
        }
    }
  free ((*p_hash_set)->NodesList);
  (*p_hash_set)->NodesList = NULL;
  free ((*p_hash_set));
  (*p_hash_set) = NULL;
}
/**
 * calculates the index of a given value
 * @param hash_func
 * @param idx
 * @param capacity
 * @param value
 * @return
 */
size_t hash_index_calculator (HashFunc hash_func, const size_t idx,
                              const unsigned int capacity, Value value)
{
  size_t hashed_value = hash_func (value);
  size_t init_value = hashed_value + (((idx) + (idx) * (idx)) / 2);
  size_t hashed_idx = (init_value) & ((capacity) - 1);
  return hashed_idx;
}
/**
 * this function helps the insertion function with recursion
 * @param hash_set
 * @param value
 * @param idx_p
 * @return
 */
int
hash_set_insert_helper
    (HashSet *hash_set, Value value, size_t *idx_p)
{
  size_t hashed_idx = hash_index_calculator (hash_set->hash_func, (*idx_p),
                                             hash_set->capacity, value);
  if ((hash_set)->NodesList[hashed_idx]->data)
    {

      *idx_p += 1;
      return hash_set_insert_helper (hash_set, value,
                                     idx_p);
    }
  else
    {

      return set_node_data (hash_set->NodesList[hashed_idx], value);
    }
}
/**
 * gets a hashset and copies it's data into an array
 * @param hash_set
 * @return
 */
Value *get_data (HashSet *hash_set)
{
  Value *data_array = malloc ((hash_set->capacity) * sizeof (Value *));
  for (unsigned int i = 0; i < hash_set->capacity; ++i)
    {
      Node *cur_node = ((hash_set->NodesList)[i]);
      if ((cur_node)->data)
        {
          data_array[i] = hash_set->value_cpy (cur_node->data);
        }
      else
        {
          data_array[i] = NULL;
        }
    }
  return data_array;
}
/**
 * this function frees the data that was allocated in the get data func
 * @param hash_set
 * @param data_array
 * @param data_size
 */
  void free_data (HashSet *hash_set, Value **data_array,
                  unsigned int data_size)
  {
    for (unsigned int i = 0; i < data_size; ++i)
      {

        if ((*data_array)[i])
          {
            hash_set->value_free (&(*data_array)[i]);
          }
      }
    free (*data_array);
    (*data_array) = NULL;

  }
/**
* this function checks if a capacity increase is needed
* @param hash_set
*/
  void check_capacity_increase (HashSet *hash_set)
  {
    unsigned int prev_cap = hash_set->capacity;
    if ((hash_set_get_load_factor (hash_set)) >= HASH_SET_MAX_LOAD_FACTOR)
      {
        unsigned int new_cap =
            HASH_SET_GROWTH_FACTOR * (hash_set->capacity);
        Value *data_array = get_data (hash_set);
        nodes_list_free (hash_set);
        Node **new_nodes_list = nodes_list_alloc (hash_set->value_cpy,
                                                  hash_set->value_cmp,
                                                  hash_set->value_free,
                                                  new_cap);
        hash_set->NodesList = new_nodes_list;
        hash_set->capacity = new_cap;
        hash_set->size = ZERO;

        for (unsigned int i = 0; i < prev_cap; ++i)
          {
            if (data_array[i])
              {
                hash_set_insert (hash_set, data_array[i]);
              }
          }
        free_data(hash_set,&data_array,prev_cap);
      }
  }
/**
 * this function checks if a capacity decrease is needed
 * @param hash_set
 */
void check_capacity_decrease(HashSet *hash_set)
{
  if ((hash_set_get_load_factor (hash_set)) <= HASH_SET_MIN_LOAD_FACTOR)
    {
      unsigned int prev_cap = hash_set->capacity;
      unsigned int new_cap =
          (hash_set->capacity) / HASH_SET_GROWTH_FACTOR;
      Value *data_array = get_data (hash_set);
      nodes_list_free (hash_set);
      Node **new_nodes_list = nodes_list_alloc (hash_set->value_cpy,
                                                hash_set->value_cmp,
                                                hash_set->value_free,
                                                new_cap);
      hash_set->NodesList = new_nodes_list;
      hash_set->capacity = new_cap;
      hash_set->size = ZERO;
      for (unsigned int i = 0; i < prev_cap; ++i)
        {
          if (data_array[i])
            {
              hash_set_insert (hash_set, data_array[i]);
            }
        }
      free_data(hash_set,&data_array,prev_cap);
    }
}

/**
 * Inserts a new Value to the hash set.
 * The function inserts *new*, *copied*, *dynamically allocated* Value,
 * @param hash_set the hash set where we want to insert the new element
 * @param value a Value we would like to add to the hashset
 * @return returns 1 for successful insertion, 0 otherwise.
 */
  int hash_set_insert (HashSet *hash_set, Value value)
  {
    if (!hash_set || !value)
      {
        return ZERO;
      }
    if (!hash_set->hash_func || !hash_set->value_cpy || !hash_set->value_cmp
        || !hash_set->value_free)
      { return ZERO; }
    if (hash_set_contains_value (hash_set, value))
      {
        return ZERO;
      }
    size_t hashed_idx = hash_index_calculator (hash_set->hash_func, ZERO,
                                               hash_set->capacity, value);
    size_t idx = get_hash_count ((hash_set->NodesList)[hashed_idx]);
    int res = hash_set_insert_helper (hash_set, value, &idx);

    if (res)
      {
        ((hash_set->NodesList)[hashed_idx])->hashCount += 1;
        hash_set->size += 1;
        check_capacity_increase(hash_set);
      }
    return res;

  }

/**
 * The function checks if the given value exists in the hash set.
 * @param hash_set a hash set.
 * @param value the value to be checked.
 * @return 1 if the value is in the hash set, 0 otherwise.
 */
  int hash_set_contains_value (HashSet *hash_set, Value value)
  {
    if (!hash_set || !value)
      {
        return ZERO;
      }
//    size_t hash_idx = hash_index_calculator (hash_set->hash_func, ZERO,
//                                             hash_set->capacity, value);

//  unsigned int hash_count = get_hash_count
//  ((hash_set->NodesList)[hash_idx]);

    for (unsigned int i = 0; i < hash_set->capacity; ++i)
      {
//        size_t curr_hash_idx = hash_index_calculator (
//            hash_set->hash_func, i, hash_set->capacity, value);
        if ((check_node ((hash_set->NodesList)[i], value))==SUCCESS)
          {
            return SUCCESS;
          }

      }
    return ZERO;
  }

/**
 * The function erases a Value.
 * @param hash_set a hash set.
 * @param value the value to be erased from the hashtable
 * @return 1 if the erasing was done successfully, 0 otherwise.
 */
  int hash_set_erase (HashSet *hash_set, Value value)
  {
    if (!hash_set || !value)
      {
        return ZERO;
      }
    if (!hash_set->hash_func || !hash_set->value_cpy || !hash_set->value_cmp
        || !hash_set->value_free)
      { return ZERO; }
    if (!hash_set_contains_value (hash_set, value))
      {
        return ZERO;
      }
    size_t hash_idx = hash_index_calculator (hash_set->hash_func, ZERO,
                                             hash_set->capacity, value);

    unsigned int hash_count = get_hash_count ((hash_set->NodesList)[hash_idx]);
    for (unsigned int i = 0; i <= hash_count; i++)
      {
        size_t curr_hash_idx = hash_index_calculator (
            hash_set->hash_func, i, hash_set->capacity, value);
        if ((check_node ((hash_set->NodesList)[curr_hash_idx], value)))
          {
            clear_node (hash_set->NodesList[curr_hash_idx]);
            hash_set->NodesList[hash_idx]->hashCount -= 1;
            hash_set->size -= 1;
            check_capacity_decrease(hash_set);
            return SUCCESS;
          }
      }
    return ZERO;
  }
/**
 * This function returns the load factor of the hash set.
 * @param hash_set a hash set.
 * @return the hash set's load factor, -1 if the function failed.
 */
  double hash_set_get_load_factor (HashSet *hash_set)
  {
    if (!hash_set)
      {
        return (double) ERR;
      }
    return (double) hash_set->size / (double) hash_set->capacity;
  }

/**
 * This function deletes all the values in the hash set.
 * @param hash_set a hash set to be cleared.
 */
  void hash_set_clear (HashSet *hash_set)
  {
    if (!hash_set)
      {
        return;
      }
    if(!(hash_set->hash_func) || !(hash_set->value_free) ||
       !(hash_set->value_cpy) || !(hash_set->value_cmp))
      {
        return;
      }
    if (hash_set->NodesList)
      {
        for (unsigned int i = 0; i < hash_set->capacity; ++i)
          {
            clear_node (hash_set->NodesList[i]);
            hash_set->NodesList[i]->hashCount = 0;
          }
        hash_set->size = 0;
      }
  }

/**
 * This function gets the element in the hashset hashed to the
 * indexed value
 * @param hash_set the hashset
 * @param value the index of the node in the hashtable
 * that we would like to have its value
 * @return the element in the hashset hashed to the indexed value
 * NULL in case of an error or an empty node or an unexpected value
 */
  Value hash_set_at (HashSet *hash_set, int value)
  {
    if (!hash_set || value < 0 || (size_t) value >= hash_set->capacity)
      {
        return NULL;
      }
    if (!(hash_set->NodesList))
      { return NULL; }
    if (hash_set->NodesList[value] == NULL)
      { return NULL; }

    return hash_set->NodesList[value]->data;
  }
