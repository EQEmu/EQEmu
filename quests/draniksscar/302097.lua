
function event_say(e)
    if e.message:findi("hail") then
        local task_link = eq.say_link("begin epic 2.0", false, "begin epic 2.0")
        e.other:Message(15, "Greetings, adventurer. I am the keeper of the Epic 2.0 collection quest. If you wish to begin, say " .. task_link .. ".")
    elseif e.message:findi("begin epic 2.0") then
        if e.other:IsTaskCompleted(990100) then
            e.other:Message(15, "You have already completed this task.")
        elseif e.other:IsTaskActive(990100) then
            e.other:Message(15, "You are already on the Epic 2.0: Fragments of War quest.")
        else
            e.other:AssignTask(990100)
            e.other:Message(15, "If your journal accepted the task, begin collecting fragments from the Omens war zones.")
        end
    end
end
