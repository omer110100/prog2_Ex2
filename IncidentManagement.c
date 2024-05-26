/*
  Omer Labinsky - 207880857
  Orel Klodin Twito - 213402233
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_NUM_RESPONDERS 100
typedef enum { false, true } bool;

typedef enum 
{
 no_error, 
 wrong_incident_type,
 duplicate_incident_id,
 out_of_range_priority,
out_of_memory,
no_incidents_to_handle,
incident_was_not_found,
error_in_responder_reading,
no_responders_at_all,
no_responder_for_incident,
no_responder_found_by_name,
}errors_e;

typedef enum
{
    violence,
    vandalism,
    ticketing_disorders,
    obstacle_on_tracks,
    zombie_attack,
    signaling_fault,
    train_technical_difficulties,
    extreme_weather_conditions,
    platform_overcrowding,
    security_alert,
    count_incident_types  //used for counting the number of incident types
} incident_type_e;

char *incident_types[] = {
    
    "violence",
    "vandalism",
    "ticketing_disorders",
    "obstacle_on_tracks",
    "zombie_attack",
    "signaling_fault",
    "train_technical_difficulties",
    "extreme_weather_conditions",
    "platform_overcrowding",
    "security_alert"
};

typedef enum
{
  administrator,
  operator,
  technician,
  analyst,
  count_roles // used for counting the number of roles
}roles_types_e;

char *role_types[] = {
    "administrator",
    "operator",
    "technician",
     "analyst"
};
typedef struct responder_s 
{
    int id;
    char* name;
    int num_handled_incidents; //keeps the number of incidents the responder has handled (closed). should be 0 at start
    int sum_handled_priorities; // sum of the priorities of the incidents the responder has handled . 
    //will be used later for average priority calculation. should be zero at start
    int incident_types[count_incident_types] ; // array whose length is the number of incident types. all values should be zeroat start. eventually , per each 
    // type it should include the number of incidents of this type the responder has handled.
    roles_types_e role;
}responder_s;

typedef struct incident_s
{
    int id; //unique id
    int priority; // the high the number the more urgent the incident
    incident_type_e type; 
    char* description;   
}incident_s;

typedef struct incident_node
{
  incident_s* incident;
  struct incident_node* next; //pointer to next incident node
}incident_node;


 int relation[count_incident_types][count_roles] = {
    {1, 0 , 0, 1},
    {1, 1, 0, 0},
    {1, 0, 1, 1},
    {1, 1, 1, 0},
    {1, 0, 1, 0},
    {1, 1, 0 , 0},
    {0, 1, 0 , 0},
    {1, 0, 1, 0},
    {1, 1, 1, 1},
    {1, 0, 0, 1}   };

typedef struct responder_statistics_s
{
 int id; //id of the responder
 int role; //role of the repsonder
 int num_handled_incidents; //total number of incidents the responder handled
 double avergae_handled_priority; //the average priority of the incidents the responder handled
 incident_type_e most_frequent_type; // the most frequent type the responder handled
}responder_statistics_s;


/////////////////////////////////////////////////////
void destroy_incident(incident_s *incident);
void print_incidents_details(incident_node *head);
void free_responders(responder_s **responders, int num_responders);
void responder_handles_incident(incident_s *incident, responder_s *responder);
void report_busiest_responder(responder_s **responders, int num_responders, responder_statistics_s *stat);
void free_incidents_list(incident_node *p_head);
void print_statistics_report(responder_statistics_s *stat);
incident_s *create_incident(int id, incident_type_e type, int priority, char *description);
errors_e add_incident(incident_node **p_head, int id, incident_type_e type, int priority, char *description);
errors_e remove_first_incident(incident_node **p_head, incident_s **p_incident);
errors_e load_responders(responder_s **responders, int *p_num_responders);
errors_e assign_responder_to_incident(incident_type_e type, responder_s **responders, int num_responders, int *ind_chosen_responder);
errors_e remove_incident_by_id(incident_node **p_head, int id, incident_s **p_incident);
errors_e handle_specific_incident(incident_node **p_head, responder_s **responders, int num_responders, int id);
errors_e handle_next_incident(incident_node **p_head, responder_s **responders, int num_responders);
errors_e report_responder_by_name(responder_s **responders, int num_responders, char *name, responder_statistics_s *stat);
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

incident_s *create_incident(int id, incident_type_e type, int priority, char *description)
{
  incident_s *newInc = malloc(sizeof(incident_s));

  if (newInc == NULL) return NULL;

  newInc->id = id;
  newInc->type = type;
  newInc->priority = priority;
  newInc->description = description;

  return newInc;
}

void destroy_incident(incident_s *incident)
{
  free(incident);
}

// the function goes over the list and prints incidents id , type, and
// description
void print_incidents_details(incident_node *head)
{
  // need to implement, use the following line for print
  // incident_node *curr;
  // printf(" incident %d %s %s\n", curr->incident->id,
  // incident_types[curr->incident->type], curr->incident->description);
  incident_node *curr = head;
  while (curr != NULL)
  {
    printf(" incident %d %s %s\n", curr->incident->id,incident_types[curr->incident->type], curr->incident->description);
    curr = curr->next;
  }
   
}

/* function: add_incident
 p_head - pointer to the head of the list
 the function  adds a new node to the list with the incident with the following
 properties id - the id of the incident type - the type of the incident priority
 - the priority of the incident description - the description of the incident
  */
errors_e add_incident(incident_node **p_head, int id, incident_type_e type, int priority, char *description)
{
  // free_incidents_list((*p_head));
  errors_e err = no_error;
  incident_node *newNode = malloc(sizeof(incident_node));
  if (newNode == NULL) return out_of_memory;

  incident_s *newInc = create_incident(id, type, priority, description);
  if (newInc == NULL)
  {
    free(newNode);
    return out_of_memory;
  }

  newNode->incident = newInc;
  newNode->next = NULL;

  if((*p_head) == NULL)
  {
    (*p_head) = newNode;
    return no_error;
  }

  incident_node *curr = (*p_head);
  incident_node *prev = (*p_head);
 
  while (curr != NULL)
  {    
    if (curr->incident->id == id)
    {
      free(newNode->incident);
      free(newNode);
      err = duplicate_incident_id;
      break;
    }
    curr = curr->next;
  }
 
  if(err != duplicate_incident_id)
  {
    curr = (*p_head);
    prev = (*p_head);

    if(curr != NULL)
    {
      if(curr->incident->priority < priority)
      {
        newNode->next = curr;
        (*p_head) = newNode;
        return err;
      }

      while (curr != NULL && curr->incident->priority >= priority)
      {
        prev = curr;
        curr = curr->next;
      }

      prev->next = newNode;
      newNode->next = curr;
    }

  }
 
  return err;
}

/* function remove_first_incident
p_head pointer to the head of the list (pointer to pointer)
p_incident pointer to pointer to incident, here the function will store the
removed incident the memory of the node should be released */
errors_e remove_first_incident(incident_node **p_head, incident_s **p_incident)
{
  if ((*p_head) == NULL) return no_incidents_to_handle;

  incident_node *newFirst = (*p_head)->next;
  (*p_incident) = (*p_head)->incident;
  free((*p_head));
  (*p_head) = newFirst;
  return no_error;
}

void free_responders(responder_s **responders, int num_responders)
{
  for (int i = 0; i < num_responders; i++)
  {
    free(responders[i]);
  }
}

/* function load_responders
responders - here responders will be stored
p_num_responders- a pointer to integer where the actual number of valid
responders is stored*/
errors_e load_responders(responder_s **responders, int *p_num_responders)
{
  FILE *fptr;
  fptr = stdin;
  char buffer[100];
  int role;
  errors_e err = no_error;

  if (feof(fptr))
    return no_responders_at_all;

  while (fgets(buffer, 100, fptr))
  {
    if((*p_num_responders) > MAX_NUM_RESPONDERS) return error_in_responder_reading;

    responder_s *new_res = malloc(sizeof(responder_s));
    if (new_res == NULL)
    {
      err = out_of_memory;
      return err;
    }

    new_res->name = (char *)malloc(20 * sizeof(char));
    if(new_res->name == NULL)
    {
      free(new_res);
      err = out_of_memory;
      return err;
    }

    new_res->role = (roles_types_e)malloc(sizeof(roles_types_e));
    if(!new_res->role)
    {
      free(new_res->name);
      free(new_res);
      err = out_of_memory;
      return err;
    }

    if(strlen(buffer)>2)
    {
      sscanf(buffer, "id:%d name:%s role:%d", &new_res->id, new_res->name, &role);

      if (!(role >= 0 && role < count_roles))
      {
        free(new_res->name);
        free(new_res);
        err = error_in_responder_reading;
      }
      else
      {
        responders[*p_num_responders] = new_res;
        responders[*p_num_responders]->id = new_res->id;
        responders[*p_num_responders]->name = new_res->name;
        responders[*p_num_responders]->role = role;
        responders[*p_num_responders]->num_handled_incidents = 0;
        responders[*p_num_responders]->sum_handled_priorities = 0;
        (*p_num_responders) += 1;
      }
    }

  }    

  return err;
}

errors_e assign_responder_to_incident(incident_type_e type, responder_s **responders, int num_responders, int *ind_chosen_responder)
{
  int index = -1;
  int num_handled_incidents = -1;

  for (int i = 0; i < num_responders; i++)
  {
    if(relation[type][responders[i]->role] == 1)
    {
      if(num_handled_incidents == -1)
      {
        index = i;
        num_handled_incidents = responders[i]->num_handled_incidents;
      }
      else if(num_handled_incidents > responders[i]->num_handled_incidents){
        index = i;
        num_handled_incidents = responders[i]->num_handled_incidents;
      }
    }    
  }

  if (index == -1) return no_responder_for_incident;

  (*ind_chosen_responder) = index;
  responders[index]->incident_types[type]++;

  return no_error;
}

// implemented for you, no need to implement
void responder_handles_incident(incident_s *incident, responder_s *responder)
{
  if (responder != NULL)
  {
    responder->num_handled_incidents++;
    responder->sum_handled_priorities += incident->priority;
    responder->incident_types[incident->type]++;
  }
}

/*function remove_incident_by-id
p_head pointer to the head of the list (pointer to pointer)
id - the id of the incident whose node should be removed
p_incident pointer to pointer to incident, here the function will store the
removed incident the memory of the node should be released inside this function
*/
errors_e remove_incident_by_id(incident_node **p_head, int id, incident_s **p_incident)
{
  incident_node *curr = (*p_head);
  incident_node *prev = (*p_head);
 
  while (curr != NULL && curr->incident->id != id)
  {
    prev = curr;
    curr = curr->next;
  }

  if(curr == NULL) return incident_was_not_found;

  prev->next = curr->next;
  (*p_incident) = curr->incident;
  free(curr);

  return no_error;
}

errors_e handle_specific_incident(incident_node **p_head, responder_s **responders, int num_responders, int id)
{
  incident_s *incident = NULL;
  int chosen_responder_ind;
  errors_e error_code = remove_incident_by_id(p_head, id, &incident);

  if (error_code != no_error) return error_code;

  error_code = assign_responder_to_incident(incident->type, responders, num_responders, &chosen_responder_ind);

  if (error_code != no_error)
  {
    destroy_incident(incident);
    return error_code;
  }

  printf("responder for incident %d is %s\n", incident->id, responders[chosen_responder_ind]->name);
  responder_handles_incident(incident, responders[chosen_responder_ind]);

  destroy_incident(incident);

  return no_error;
}

errors_e handle_next_incident(incident_node **p_head, responder_s **responders, int num_responders)
{

  incident_s *incident = NULL;
  int chosen_responder_ind;
  errors_e error_code = remove_first_incident(p_head, &incident);

  if (error_code != no_error) return error_code;

  error_code = assign_responder_to_incident(incident->type, responders, num_responders, &chosen_responder_ind);

  if (error_code != no_error)
  {
    destroy_incident(incident);
    return error_code;
  }

  responder_s *responder = responders[chosen_responder_ind];

  printf("responder for incident %d of type %d is %s of role %s\n", incident->id, incident->type, responder->name, role_types[responder->role]);
  responder_handles_incident(incident, responder);

  destroy_incident(incident);
  return no_error;
}

void report_busiest_responder(responder_s **responders, int num_responders, responder_statistics_s *stat)
{
  int biggest = 0;
  int biggestIndex = -1;
  double avergae_handled_priority = 0;
  int most_frequent_type = 0;
  int most_frequent_type_index = 0;

  for (int i = 0; i < num_responders; i++)
  {
    if(responders[i]->num_handled_incidents > biggest)
    {
      biggest = responders[i]->num_handled_incidents;
      biggestIndex = i;
    }
  }

  for (size_t i = 0; i < count_incident_types; i++)
  {
    if(responders[biggestIndex]->incident_types[i] > most_frequent_type)
    {
      most_frequent_type = responders[biggestIndex]->incident_types[i];
      most_frequent_type_index = i;
    }
  }
  

  avergae_handled_priority = responders[biggestIndex]->sum_handled_priorities / (responders[biggestIndex]->num_handled_incidents ? responders[biggestIndex]->num_handled_incidents : 1);
  stat->id = responders[biggestIndex]->id;
  stat->avergae_handled_priority = avergae_handled_priority;
  stat->most_frequent_type = most_frequent_type_index;
  stat->num_handled_incidents = responders[biggestIndex]->num_handled_incidents;
  stat->role = responders[biggestIndex]->role;
 
}

errors_e report_responder_by_name(responder_s **responders, int num_responders, char *name, responder_statistics_s *stat)
{
  int responderIndex = -1;
  double avergae_handled_priority = 0;
  int most_frequent_type = 0;
  int most_frequent_type_index = 0;
  for (int i = 0; i < num_responders; i++)
  {
    if( !strcmp(responders[i]->name, name) )
    {
      
      responderIndex = i;
      break;
    }
  }

  if(responderIndex == -1) return no_responder_found_by_name;

  for (size_t i = 0; i < count_incident_types; i++)
  {
    if(responders[responderIndex]->incident_types[i] > most_frequent_type)
    {
      most_frequent_type = responders[responderIndex]->incident_types[i];
      most_frequent_type_index = i;
    }
  }
  
  avergae_handled_priority = responders[responderIndex]->sum_handled_priorities / (responders[responderIndex]->num_handled_incidents ? responders[responderIndex]->num_handled_incidents : 1);
  stat->id = responders[responderIndex]->id;
  stat->avergae_handled_priority = avergae_handled_priority;
  stat->most_frequent_type = most_frequent_type_index;
  stat->num_handled_incidents = responders[responderIndex]->num_handled_incidents;
  stat->role = responders[responderIndex]->role;

  return no_error;
}

void free_incidents_list(incident_node *p_head)
{
   struct incident_node* tmp;

   while (p_head != NULL)
    {
       tmp = p_head;
       p_head = p_head->next;
       free(tmp->incident);
       free(tmp);
    }
}

void print_statistics_report(responder_statistics_s *stat)
{
  printf("printing statistics of user with id %d and role %s\n", stat->id, role_types[stat->role]);
  printf("num of handled indices %d\n", stat->num_handled_incidents);
  printf("average handled priority %f\n", stat->avergae_handled_priority);
  printf("most frequent handled incident type %s\n", incident_types[stat->most_frequent_type]);
}