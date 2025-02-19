import { Component } from '@angular/core';

@Component({
  selector: 'app-navbar',
  standalone: true,
  imports: [],
  templateUrl: './nav.component.html',
  styleUrl: './nav.component.scss',
})
export class NavbarComponent {
  navBarClick() {
    document.getElementById('navMenu')?.classList.toggle('active');
    document.getElementById('menuBtn')?.classList.toggle('active');
  }
}
