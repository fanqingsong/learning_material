import java.awt.*;

public class demo_dialog extends Dialog {
    public demo_dialog(Frame parent) {
        super(parent, "Java!", true);

        Panel p1 = new Panel();
        p1.setLayout(new FlowLayout());
        p1.add(new Button("Close"));
        add("South", p1);

        Panel p2 = new Panel();
        p2.setLayout(new FlowLayout());
        p2.add(new Label("We love Linux!"));
        add("Center", p2);

        resize(200, 100);
    }

    public boolean action(Event event, Object object) {
        if (object.equals("Close")) {
            dispose();
            return true;
        }
        return false;
    }


    public boolean handleEvent(Event event) {
      if (event.id == Event.WINDOW_DESTROY) {
            dispose();
            return true;
        }
        return super.handleEvent(event);
    }
}
