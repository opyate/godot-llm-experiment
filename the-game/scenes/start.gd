extends Node

signal next_please

@onready var dialogue = %dialogue
@onready var p1 = %Person1
@onready var p2 = %Person2


func _ready():
	next_please.connect(next_person_say_something)
	
	# kick off
	call_deferred("next_person_say_something")


func next_person_say_something():
	var response = dialogue.get_next_person_line()
	p1.emit_signal("say", self, response)
	p2.emit_signal("say", self, response)
