import java.util.concurrent.Delayed;
import java.util.concurrent.TimeUnit;

public class Task implements Delayed {
    public TaskType type;
    public Particle p;
    public Sprite s;
    public float spriteDx;
    public float spriteDy;
    public float currFramerate;

    public Task(Particle p, float currFramerate){
        this.type = TaskType.UPDATE_PARTICLE;
        this.p = p;
        this.currFramerate = currFramerate;
    }

    public Task(Sprite s, float dx, float dy, float currFramerate){
        this.type = TaskType.MOVE_USER;
        this.s = s;
        this.spriteDx = dx;
        this.spriteDy = dy;
        this.currFramerate = currFramerate;
    }

    @Override
    public long getDelay(TimeUnit unit) {
        return 0;
    }

    @Override
    public int compareTo(Delayed o) {
        return 0;
    }
}
