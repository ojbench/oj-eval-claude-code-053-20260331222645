#pragma once
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>
// You are NOT allowed to add any headers
// without the permission of TAs.
namespace Grammar {
class NFA;
NFA MakeStar(const char &character);
NFA MakePlus(const char &character);
NFA MakeQuestion(const char &character);
NFA Concatenate(const NFA &nfa1, const NFA &nfa2);
NFA Union(const NFA &nfa1, const NFA &nfa2);
NFA MakeSimple(const char &character);
/*!
  \brief This class is used to store the type of the transition.
  \details You can use it like this:
  \code
      TransitionType type = TransitionType::Epsilon;
      TransitionType type2 = TransitionType::a;
      TransitionType type3 = TransitionType::b;
  \endcode
*/
enum class TransitionType { Epsilon, a, b };
struct Transition {
  /*!
    \brief value is used to store which character to match.
  */
  TransitionType type;
  /*!
    \brief This is used to store which node the transition is going to.
  */
  int to;
  Transition(TransitionType type, int to) : type(type), to(to) {}
};
class NFA {
private:
  /*!
    \brief This is used to store the start state of the NFA.
  */
  int start;
  /*!
    \brief This is used to store the end state of the NFA.
  */
  std::unordered_set<int> ends;
  /*!
    \brief This is used to store the transitions of the NFA.
    \details For example, transitions[3] stores all the transitions beginning
    with \details state 3.
  */
  std::vector<std::vector<Transition>> transitions;

public:
  NFA() = default;
  ~NFA() = default;
  /*!
    \brief Get the epsilon closure of a state.
    \param state The state to get the epsilon closure of.
    \return The epsilon closure of the state.
*/
  std::unordered_set<int>
  GetEpsilonClosure(std::unordered_set<int> states) const {
    std::unordered_set<int> closure;
    std::queue<int> queue;
    for (const auto &state : states) {
      if (closure.find(state) != closure.end())
        continue;
      queue.push(state);
      closure.insert(state);
    }
    while (!queue.empty()) {
      int current = queue.front();
      queue.pop();
      for (const auto &transition : transitions[current]) {
        if (transition.type == TransitionType::Epsilon) {
          if (closure.find(transition.to) == closure.end()) {
            queue.push(transition.to);
            closure.insert(transition.to);
          }
        }
      }
    }
    return closure;
  }
  /*!
    \brief Advance the NFA to the next states.
    \param current_states The current states of the NFA.
    \param character The character to match.
    \return The next states of the NFA.
  */
  std::unordered_set<int> Advance(std::unordered_set<int> current_states,
                                  char character) const {
    // Get epsilon closure of current states
    std::unordered_set<int> closure = GetEpsilonClosure(current_states);

    // Get next states by matching character
    std::unordered_set<int> next_states;
    TransitionType type = (character == 'a') ? TransitionType::a : TransitionType::b;

    for (int state : closure) {
      for (const auto &transition : transitions[state]) {
        if (transition.type == type) {
          next_states.insert(transition.to);
        }
      }
    }

    // Get epsilon closure of next states
    return GetEpsilonClosure(next_states);
  }

  /*!
    \brief Check if a state is accepted.
    \param state The state to check.
    \return true if the state is accepted, false otherwise.
  */
  bool IsAccepted(int state) const { return ends.find(state) != ends.end(); }

  /*!
    \brief Return the start state.
    \return The start state.
  */

  int GetStart() const { return start; }

  friend NFA MakeStar(const char &character);
  friend NFA MakePlus(const char &character);
  friend NFA MakeQuestion(const char &character);
  friend NFA MakeSimple(const char &character);
  friend NFA Concatenate(const NFA &nfa1, const NFA &nfa2);
  friend NFA Union(const NFA &nfa1, const NFA &nfa2);
};
class RegexChecker {
private:
  /*!
    \brief This is used to store the regex string.
  */
  NFA nfa;

public:
  /*!
    \brief To check if the string is accepted by the regex.
    \param str The string to be checked.
    \return true if the string is accepted by the regex, false otherwise.
  */
  bool Check(const std::string &str) const {
    std::unordered_set<int> current_states;
    current_states.insert(nfa.GetStart());

    // Get epsilon closure of start state
    current_states = nfa.GetEpsilonClosure(current_states);

    // Process each character
    for (char c : str) {
      current_states = nfa.Advance(current_states, c);
      if (current_states.empty()) {
        return false;
      }
    }

    // Check if any current state is an accept state
    for (int state : current_states) {
      if (nfa.IsAccepted(state)) {
        return true;
      }
    }

    return false;
  }
  /*!
    \brief This is used to build the NFA from the regex string.
  */
  RegexChecker(const std::string &regex) {
    // Parse the regex string and build NFA
    // First split by '|' to handle unions (lowest priority)
    std::vector<std::string> union_parts;
    std::string current_part;

    for (char c : regex) {
      if (c == '|') {
        union_parts.push_back(current_part);
        current_part.clear();
      } else {
        current_part += c;
      }
    }
    union_parts.push_back(current_part);

    // Build NFA for each union part
    std::vector<NFA> union_nfas;
    for (const auto &part : union_parts) {
      // Parse each part (concatenation)
      NFA part_nfa;
      bool has_nfa = false;

      for (size_t i = 0; i < part.size(); i++) {
        char c = part[i];
        NFA current_nfa;

        if (c == 'a' || c == 'b') {
          // Check if next character is a modifier
          if (i + 1 < part.size()) {
            char next = part[i + 1];
            if (next == '*') {
              current_nfa = MakeStar(c);
              i++; // Skip next character
            } else if (next == '+') {
              current_nfa = MakePlus(c);
              i++;
            } else if (next == '?') {
              current_nfa = MakeQuestion(c);
              i++;
            } else {
              current_nfa = MakeSimple(c);
            }
          } else {
            current_nfa = MakeSimple(c);
          }

          // Concatenate with previous
          if (has_nfa) {
            part_nfa = Concatenate(part_nfa, current_nfa);
          } else {
            part_nfa = current_nfa;
            has_nfa = true;
          }
        }
      }

      if (has_nfa) {
        union_nfas.push_back(part_nfa);
      }
    }

    // Union all parts
    if (!union_nfas.empty()) {
      nfa = union_nfas[0];
      for (size_t i = 1; i < union_nfas.size(); i++) {
        nfa = Union(nfa, union_nfas[i]);
      }
    }
  }
};

/*!
  \brief Return a NFA to match a* or b*.
  \param character The character to match.
  \details This function will create a NFA that matches a* or b*.
*/
NFA MakeStar(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(0);
  nfa.transitions.push_back(std::vector<Transition>());
  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 0});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 0});
  }
  return nfa;
}
/*!
  \brief Return a NFA to match a+ or b+.
  \param character The character to match.
  \details This function will create a NFA that matches a+ or b+.
*/
NFA MakePlus(const char &character) {
  // a+ means at least one 'a', equivalent to a followed by a*
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(1);
  nfa.transitions.resize(2);

  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
    nfa.transitions[1].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
    nfa.transitions[1].push_back({TransitionType::b, 1});
  }

  return nfa;
}
/*!
  \brief Return a NFA to match a? or b?.
  \param character The character to match.
  \details This function will create a NFA that matches a? or b?.
*/
NFA MakeQuestion(const char &character) {
  // a? means zero or one 'a'
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(0);
  nfa.ends.insert(1);
  nfa.transitions.resize(2);

  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
  }

  return nfa;
}

/*!
  \brief Return a NFA to match ...(the first part) ...(the second part).
  \param nfa1 The first NFA to match, for example, it is a.
  \param nfa2 The second NFA to match, for example, it is b.
  \details This function will create a NFA that matches ab.
*/
NFA Concatenate(const NFA &nfa1, const NFA &nfa2) {
  NFA result;
  int offset = nfa1.transitions.size();

  // Copy nfa1's transitions
  result.transitions = nfa1.transitions;
  result.start = nfa1.start;

  // Add nfa2's transitions with offset
  for (const auto &trans_list : nfa2.transitions) {
    std::vector<Transition> new_trans_list;
    for (const auto &trans : trans_list) {
      new_trans_list.push_back({trans.type, trans.to + offset});
    }
    result.transitions.push_back(new_trans_list);
  }

  // Add epsilon transitions from nfa1's end states to nfa2's start state
  for (int end : nfa1.ends) {
    result.transitions[end].push_back({TransitionType::Epsilon, nfa2.start + offset});
  }

  // Set end states to nfa2's end states (with offset)
  result.ends.clear();
  for (int end : nfa2.ends) {
    result.ends.insert(end + offset);
  }

  return result;
}
/*!
  \brief Return a NFA to match ... | ...
  \param nfa1 The first NFA to match, for example, it is a.
  \param nfa2 The second NFA to match, for example, it is b.
  \details This function will create a NFA that matches a|b.
*/
NFA Union(const NFA &nfa1, const NFA &nfa2) {
  NFA result;

  // New start state
  result.start = 0;
  result.transitions.resize(1);

  int offset1 = 1;
  int offset2 = 1 + nfa1.transitions.size();

  // Add epsilon transitions from new start to both nfa starts
  result.transitions[0].push_back({TransitionType::Epsilon, nfa1.start + offset1});
  result.transitions[0].push_back({TransitionType::Epsilon, nfa2.start + offset2});

  // Copy nfa1's transitions with offset
  for (const auto &trans_list : nfa1.transitions) {
    std::vector<Transition> new_trans_list;
    for (const auto &trans : trans_list) {
      new_trans_list.push_back({trans.type, trans.to + offset1});
    }
    result.transitions.push_back(new_trans_list);
  }

  // Copy nfa2's transitions with offset
  for (const auto &trans_list : nfa2.transitions) {
    std::vector<Transition> new_trans_list;
    for (const auto &trans : trans_list) {
      new_trans_list.push_back({trans.type, trans.to + offset2});
    }
    result.transitions.push_back(new_trans_list);
  }

  // Set end states (both nfa1 and nfa2 end states with offsets)
  for (int end : nfa1.ends) {
    result.ends.insert(end + offset1);
  }
  for (int end : nfa2.ends) {
    result.ends.insert(end + offset2);
  }

  return result;
}

/*!
  \brief Return a NFA to match a, or return a NFA to match b.
  \param character The character to match.
  \return The required NFA.
*/
NFA MakeSimple(const char &character) {
  // Simple NFA: start state -> character -> end state
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(1);
  nfa.transitions.resize(2);

  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
  }

  return nfa;
}
} // namespace Grammar
