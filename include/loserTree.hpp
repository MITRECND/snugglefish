/*
Approved for Public Release; Distribution Unlimited: 13-1937


Copyright (c) 2014 The MITRE Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

*/

#ifndef LOSERTREE
#define LOSERTREE

#include <stdint.h>
#include <vector>
#include <cmath>

namespace snugglefish {

    using namespace std;

    //Class Declarations

    template<class comparable>
    class playerNode;
    template<class comparable>
    class matchNode;

    template<class comparable>
    struct treeElement {
        comparable value;
        playerNode<comparable>*   pNodePtr; //pointer to the player node where this should reside

        treeElement(comparable value, playerNode<comparable>* pNodePtr) {
            this-> value = value; this->pNodePtr = pNodePtr;
        }
    
        treeElement(){;}
    };

    template<class comparable>
    class treeNode{
        public:
            virtual comparable getValue() = 0; //pure virtual function
            virtual treeElement<comparable> getTreeElement() = 0;
            virtual uint8_t isPlayer() = 0;
    };

    template<class comparable>
    class playerNode: public treeNode<comparable> {
        public:
            playerNode(matchNode<comparable>* parent, std::vector<comparable>* playerList, uint32_t playerListSize, uint32_t playerId, comparable sentinel);
            comparable getValue();

            treeElement<comparable> getTreeElement() {return treeElement<comparable>(this->getValue(), this);}
            void advancePlayer() { index_id++;}
            uint8_t isPlayer() { return 1; }
            const uint32_t getId() { return this->position; }

            matchNode<comparable>* parent;

        private:
            std::vector<comparable>* player; //player for this player node
            uint32_t position; //where in the playerlist did you get the above node from?
            uint32_t index_id; //where in the list are you
            comparable sentinel;
    };


    template<class comparable>
    class matchNode: public treeNode<comparable> {
        public:
            comparable getValue() {return element.value;}
            treeElement<comparable> getTreeElement() { return this->element; }
            uint8_t isPlayer() {return 0;}

            matchNode<comparable>* parent;
            treeNode<comparable>* left;
            treeNode<comparable>* right;
            treeElement<comparable> element;
            uint32_t depth;
    };

    template<class comparable>
    class loserTree{
        public:
            loserTree(std::vector<comparable>* playerList, uint32_t listSize, comparable sentinel);
            //TODO getWinner renamed not public
            comparable getWinnerValue();
            uint32_t getWinnerId();
            void playNextMatch();
            ~loserTree();


        private:
            void removeAndPlay(treeElement<comparable>* winner);
            void buildTree();
            void destroyChildren(matchNode<comparable>* node);
            void playUp(matchNode<comparable>* node, treeElement<comparable> winner, matchNode<comparable>* winnerNode);
            treeElement<comparable> getMatchWinner(treeElement<comparable> & loser);
            treeElement<comparable> buildChildren(matchNode<comparable>* node);
            treeElement<comparable> getWinner();

            std::vector<comparable>* playerList; //array of pointers to vectors
            uint32_t playerListSize;
            uint32_t playerId; //to keep track of which vectors we've assigned

            uint32_t match_depth;
            treeElement<comparable> winner;
            matchNode<comparable>* root;
            comparable sentinel;
    };

//DEFINITIONS

    template<class comparable>
    loserTree<comparable>::loserTree(vector<comparable>* playerList, uint32_t listSize, comparable sentinel){
        //Match depth, not including players is log base 2 (listSize)
        this->sentinel = sentinel;
        this->playerList = playerList;
        this->playerListSize = listSize;
        this->playerId = 0; 
        this->match_depth = ceil(log10((double) listSize) / log10((double) 2));


        this->root = new matchNode<comparable>;
        this->root->parent = 0;
        this->root->depth = 1;

        this-> winner = this->buildChildren(this->root);

    }


    template<class comparable>
    loserTree<comparable>::~loserTree(){
        //Recursively destroy tree
        this->destroyChildren(this->root);
    }


    template<class comparable>
    treeElement<comparable> loserTree<comparable>::getWinner(){
        return this->winner;
    }

    template<class comparable>
    uint32_t loserTree<comparable>::getWinnerId(){
        return this->winner.pNodePtr->getId();
    }

    template<class comparable>
    comparable loserTree<comparable>::getWinnerValue(){
        return this->winner.value;
    }

    template<class comparable>
    void loserTree<comparable>::playNextMatch(){
        if (this->winner.pNodePtr != 0){
            this->winner.pNodePtr->advancePlayer();
            this->playUp(this->winner.pNodePtr->parent, this->winner.pNodePtr->getTreeElement(), (matchNode<comparable>*)this->winner.pNodePtr);
        }
    }

    //Starting at the deepest match node, go upwards
    template<class comparable>
    void loserTree<comparable>::playUp(matchNode<comparable>* node, treeElement<comparable> winner, matchNode<comparable>* winnerNode){
        if(node->depth == 1){  //root node
            if(winner.value > node->element.value ||
                (winner.value == node->element.value &&
                    node->right == winnerNode)){
                this->winner = node->element;
                node->element = winner;
            }else{
                this->winner = winner;
            }

        }else{
            //Compare winner received with stored loser rescurse upwards
            if(winner.value > node->element.value || 
                (winner.value == node->element.value && 
                    node->right == winnerNode)){//new loser
                this->playUp(node->parent, node->element, node);
                node->element = winner;
            }else{
                this->playUp(node->parent, winner, node);
            }
        }
    }

    template<class comparable>
    void loserTree<comparable>::destroyChildren(matchNode<comparable>* node){
        if (node->depth == this->match_depth){
            //clean up player list?
        }else{
            if(!node->isPlayer()){
                destroyChildren((matchNode<comparable>*)node->left);
                if (node->right)
                    destroyChildren((matchNode<comparable>*)node->right);
            }
        }

        if(!node->isPlayer()){
            delete node->left;
            if (node->right)
                delete node->right;
        }
    }

    template<class comparable>
    treeElement<comparable> loserTree<comparable>::buildChildren(matchNode<comparable>* node){
        treeElement<comparable> right,left;

        if (node->depth == this->match_depth || 
                this->playerId >= (this->playerListSize - 1)) { //Children are player nodes
            node->left = new playerNode<comparable>(node, this->playerList, 
                    this->playerListSize, this->playerId++, this->sentinel);
            node->right = new playerNode<comparable>(node, this->playerList, 
                    this->playerListSize, this->playerId++, this->sentinel);

            left.value = node->left->getValue();
            left.pNodePtr = (playerNode<comparable>*)node->left;

            right.value = node->right->getValue();
            right.pNodePtr = (playerNode<comparable>*)node->right;

        }else{//Regular match node where children are matches
            node->left = (treeNode<comparable>*) new matchNode<comparable>;
            matchNode<comparable>* leftMatch = (matchNode<comparable>*) node->left;
            leftMatch->parent = node;
            leftMatch->depth = node->depth + 1;

            left = this->buildChildren((matchNode<comparable>*) node->left);

            if(this->playerId >= this->playerListSize){
            //there are no more lists to sort, seed this instead
                node->right = 0;
                right = treeElement<comparable>(this->sentinel, 0);
            }else{
                node->right = (treeNode<comparable>*) new matchNode<comparable>;
                matchNode<comparable>* rightMatch = (matchNode<comparable>*) node->right;
                rightMatch->parent = node;
                rightMatch->depth = node->depth + 1;
                right = this->buildChildren((matchNode<comparable>*) node->right);
            }

        }

        if (right.value < left.value) { //right is the winner
            node->element = left; //store the loser
            return right; //return the winner
        }else{//if right => left it loses
            node->element = right;
            return left;
        } 
    }




    template<class comparable>
    playerNode<comparable>::playerNode(matchNode<comparable>* parent, vector<comparable>* playerList, uint32_t playerListSize,  uint32_t playerId, comparable sentinel){
        this->parent = parent;
        this->sentinel = sentinel;
        if(playerId >= playerListSize){//uneven 
            this->player = 0;    
        }else{
            this->player = & (playerList[playerId]);
            this->position = playerId;
        }
        this->index_id = 0;
    }


    template<class comparable>
    comparable playerNode<comparable>::getValue(){
        if(player && index_id < ((std::vector<comparable>)(*player)).size()) 
            return ((std::vector<comparable>)(*player))[index_id];
        else
             return this->sentinel;
    }






}


#endif
