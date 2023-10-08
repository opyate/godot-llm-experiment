extends Node2D

signal next_please

@onready var dialogue = $dialogue
@onready var p1 = $Bg/Person1
@onready var p2 = $Bg/Person2


func _ready():
	$Bg.position.x = int(ProjectSettings.get_setting("display/window/size/viewport_width")) / 2
	$Bg.position.y = int(ProjectSettings.get_setting("display/window/size/viewport_height")) / 2
	
	next_please.connect(next_person_say_something)
	
	# kick off
	next_person_say_something()

func next_person_say_something():
	var response = dialogue.get_next_person_line()
	p1.emit_signal("say", self, response)
	p2.emit_signal("say", self, response)
