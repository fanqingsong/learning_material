import java.awt.*;

public class blp_demo extends Frame {
        private MenuBar my_menu_bar;
        private Menu file_menu;
        private Button ok_button;
        private String fname = "";
        private Panel my_panel1;
        private Panel my_panel2;
        private Panel my_panel3;
        private TextField text_field;
        private Checkbox likes_cats;
        private Checkbox likes_dogs;
        private CheckboxGroup cbg;
        private Checkbox c_cb;
        private Checkbox t_cb;
        private Checkbox b_cb;
        private List people_list;
        boolean cats = false;
        boolean dogs = false;


        blp_demo(String title) {
            super(title);

            my_panel1 = new Panel();
            my_panel1.setLayout(new FlowLayout());
            my_panel2 = new Panel();
            my_panel2.setLayout(new FlowLayout());
            my_panel3 = new Panel();
            my_panel3.setLayout(new FlowLayout());

            // Menu definition
            my_menu_bar = new MenuBar();
            this.setMenuBar(my_menu_bar);
            file_menu = new Menu("File");
            file_menu.add(new MenuItem("Open"));
            file_menu.add(new MenuItem("Do Nothing"));
            file_menu.add(new MenuItem("About"));
            file_menu.addSeparator();
            file_menu.add(new MenuItem("Exit"));
            my_menu_bar.add(file_menu);
            my_menu_bar.add(new Menu("Help"));

            // A text field
            text_field = new TextField("A file name?", 35);
            my_panel1.add(text_field);

            // Check boxes
            likes_cats = new Checkbox("Likes cats");
            likes_dogs = new Checkbox("Likes dogs");
            my_panel2.add(new Label("Pet status"));
            my_panel2.add(likes_cats);
            my_panel2.add(likes_dogs);

            // Ordinary buttons
            my_panel2.add(new Button("Press  "));
            my_panel2.add(new Button("Release"));

            // Radio Buttons
            my_panel3.add(new Label("Favorite drink"));
            cbg = new CheckboxGroup();
            my_panel3.add(c_cb = new Checkbox("Coffee", cbg, false));
            my_panel3.add(t_cb = new Checkbox("Tea   ", cbg, true));
            my_panel3.add(b_cb = new Checkbox("Beer  ", cbg, false));


            // A list of names, with a label
            my_panel1.add(new Label("People at Wrox"));
            people_list = new List(4, true);
            people_list.addItem("Dave");
            people_list.addItem("Julian");
            people_list.addItem("Tim");
            people_list.addItem("Chris");
            people_list.addItem("Martin");
            people_list.addItem("Gordon");
            my_panel1.add(people_list);

            // Add the panels to the frame
            add("West",  my_panel1);
            add("North", my_panel3);
            add("South", my_panel2);
        }


        // This overloads the default method to allow event processing
        public boolean handleEvent(Event event) {
            boolean event_processed = false;
            if (event.id == Event.ACTION_EVENT) {

                // Check for menu items
                if (event.target instanceof MenuItem) {
                    if (((String)event.arg).equals("Exit")) {
                        System.exit(0);
                   }
                   if (((String)event.arg).equals("About")) {
                        Dialog dialog = new demo_dialog(this);
                        dialog.show();
                    }

                    if (((String)event.arg).equals("Open")) {
                        FileDialog fd = new FileDialog(this, "File open", FileDialog.LOAD);
                        fd.show();
                        fname = fd.getFile();
                        if (fname == null) text_field.setText("No file selected");
                        else text_field.setText(fname);

                    }
                    event_processed = true;
                }

                // Check for ordinary buttons
                if (event.target instanceof Button) {
                    if (((String)event.arg).equals("Press  ")) {
                        text_field.setText("press");
                    }
                    if (((String)event.arg).equals("Release")) {
                        text_field.setText("release");
                    }
                    event_processed = true;
                }


                // If an Checkbox update the information
                if (event.target instanceof Checkbox) {
                    boolean update_animals = false;
                    if (event.target.equals(likes_cats)) {
                        cats = cats ? false : true;
                        update_animals = true;
                    }
                    if (event.target.equals(likes_dogs)) {
                       dogs = dogs ? false : true;
                       update_animals = true;
                    }
                    if (update_animals) {
                        StringBuffer likes_animals = new StringBuffer("Likes: ");
                        if (cats) likes_animals.append("cats ");
                        if (dogs) likes_animals.append("dogs ");
                        text_field.setText(likes_animals.toString());
                        return true;
                    }
                    else {
                        if (event.target.equals(c_cb)) text_field.setText("Coffee");
                        if (event.target.equals(t_cb)) text_field.setText("Tea");
                        if (event.target.equals(b_cb)) text_field.setText("Beer");
                    }
                    event_processed = true;
               }

            }
            else if ((event.id == Event.LIST_SELECT) ||
                      (event.id == Event.LIST_DESELECT)) {
                if (event.target.equals(people_list)) {
                    String items[] = people_list.getSelectedItems();
                    String display_text = "";
                    for (int i = 0; i < items.length; i++) display_text += items[i] + " ";
                    text_field.setText(display_text);
                    event_processed = true;
                }
            }
            else if (event.id == Event.WINDOW_DESTROY) {
                System.exit(0);
                event_processed = true;
            }

            if (event_processed) return true;
            else return super.handleEvent(event);

        }

        public static void main(String[] a) {
            Frame f = new blp_demo("Demo for Beginning Linux Programming");
            f.resize(500,250);
            f.show();

        }
}





