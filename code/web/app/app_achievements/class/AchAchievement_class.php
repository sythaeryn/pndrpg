<?php
	class AchAchievement extends AchList {
		#########################
		# PHP 5.3 compatible
		# InDev_trait replaces this in PHP 5.4
		protected $dev;

		function inDev() {
			return ($this->dev == 1);
		}

		function getDev() {
			return $this->dev;
		}

		function setInDev($tf) {
			if($tf == true) {
				$this->setDev(1);
			}
			else {
				$this->setDev(0);
			}

			$this->update();
		}

		function setDev($d) {
			$this->dev = $d;
		}
		#########################

		protected $parent_id;
		protected $category;
		protected $tie_race;
		protected $tie_civ;
		protected $tie_cult;
		protected $image;
		protected $name;
		protected $template;
		protected $sticky;

		function AchAchievement($data,$parent) {
			global $DBc,$_USER;

			parent::__construct();
			
			$this->setParent($parent);
			$this->setID($data['aa_id']);
			$this->parent_id = $data['aa_parent'];
			$this->category = $data['aa_category'];
			$this->tie_race = $data['aa_tie_race'];
			$this->tie_civ = $data['aa_tie_civ'];
			$this->tie_cult = $data['aa_tie_cult'];
			$this->image = $data['aa_image'];
			$this->name = $data['aal_name'];
			$this->template = $data['aal_template'];
			$this->dev = $data['aa_dev'];
			$this->sticky = $data['aa_sticky'];

			$res = $DBc->sqlQuery("SELECT * FROM ach_task LEFT JOIN (ach_task_lang) ON (atl_lang='".$_USER->getLang()."' AND atl_task=at_id) LEFT JOIN (ach_player_task) ON (apt_task=at_id AND apt_player='".$_USER->getID()."') WHERE at_achievement='".$this->id."' ORDER by at_torder ASC");
			
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$tmp = $this->makeChild($res[$i]);

				if($tmp->isDone()) {
					$this->addDone($tmp);
				}
				else {
					$this->addOpen($tmp);
				}
			}
		}

		function parentDone() {
			if($this->parent_id == null) {
				return true;
			}
			else {
				$p = $this->parent->getChildDataByID($this->parent_id);

				return ($p->hasOpen() == false);
			}
		}

		protected function makeChild($a) {
			return new AchTask($a,$this);
		}

		function unlockedByParent() {
			if($this->parent_id != null) {
				$tmp = $this->parent->getChildByID($this->parent_id);
				return ($tmp->hasOpen() == false);
			}

			return true;
		}

		function getParentID() {
			return $this->parent_id;
		}

		function getTieRace() {
			return $this->tie_race;
		}

		function getTieCiv() {
			return $this->tie_civ;
		}

		function getTieCult() {
			return $this->tie_cult;
		}

		function getImage() {
			return $this->image;
		}

		function getName() {
			return $this->name;
		}

		function getValueDone() {
			$val = 0;
			$iter = $this->getDone();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$val += $curr->getValue();
			}
			return $val;
		}

		function getValueOpen() {
			$iter = $this->getOpen();
			if($iter->hasNext()) {
				$curr = $iter->getNext();
				return $curr->getValue();
			}
			return 0;
		}

		function fillTemplate($insert = array()) {
			if($this->template == null) {
				return implode(";",$insert);
			}
			else {
				$tmp = $this->template;
				$match = array();
				preg_match_all('#\[([0-9]+)\]#', $this->template, $match);
				foreach($match[0] as $key=>$elem) {
					$tmp = str_replace("[".$match[1][$key]."]",$insert[$key],$tmp);
				}
				return $tmp;
			}
		}
		
		function getTemplate() {
			return $this->template;
		}

		function getCategory() {
			return $this->category;
		}

		function getSticky() {
			return $this->sticky;
		}

		function isSticky() {
			return ($this->sticky == 1);
		}

		function isHeroic() {
			return $this->parent->isHeroic();
		}
		
	}
?>