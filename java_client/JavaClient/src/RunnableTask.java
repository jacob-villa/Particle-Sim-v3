public class RunnableTask implements Runnable{
    private final Task task;

    RunnableTask(Task task){
        this.task = task;
    }
    @Override
    public void run() {
        switch (this.task.type){
            case UPDATE_PARTICLE -> task.p.updatePosition(task.currFramerate);
            case MOVE_USER -> task.s.move(task.spriteDx, task.spriteDy, task.currFramerate);
        }
    }
}
