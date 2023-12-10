#include <stdio.h>
#include <allegro5/allegro.h>
//#include <allegro5/allegro_joystick.h>

int main() {
    ALLEGRO_JOYSTICK *analogJoystick = NULL;
    ALLEGRO_JOYSTICK *digitalJoystick = NULL;

    if (!al_init()) {
        fprintf(stderr, "Błąd inicjalizacji Allegro.\n");
        return -1;
    }

    if (!al_install_joystick()) {
        fprintf(stderr, "Błąd inicjalizacji obsługi joysticka.\n");
        return -1;
    }

    if (al_get_num_joysticks() < 1) {
        fprintf(stderr, "Brak podłączonego joysticka.\n");
        return -1;
    }

//    printf("Liczba joy: %d\n",al_get_num_joysticks());
    analogJoystick = al_get_joystick(0);
    digitalJoystick = al_get_joystick(1);

    if (!analogJoystick && !digitalJoystick) {
        fprintf(stderr, "Błąd pobierania uchwytu do joysticków.\n");
        return -1;
    }

    printf("Analogowy joystick znaleziony: %s\n", al_get_joystick_name(analogJoystick));
//    printf("Zwykły joystick znaleziony: %s\n", al_get_joystick_name(digitalJoystick));

    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue(); // Tworzenie kolejki zdarzeń
    if (!event_queue) {
        fprintf(stderr, "Błąd tworzenia kolejki zdarzeń.\n");
        return -1;
    }

    al_register_event_source(event_queue, al_get_joystick_event_source()); // Rejestracja źródła zdarzeń joysticka

    while (1) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev); // Oczekiwanie na zdarzenie w kolejce
        printf ("ev: %d stick: %d pos: %f axis: %d btn: %d\n",ev.type, ev.joystick.stick, ev.joystick.axis, ev.joystick.pos,ev.joystick.button);


    }

    al_destroy_event_queue(event_queue); // Zniszczenie kolejki zdarzeń
    al_uninstall_joystick();
    return 0;
}
