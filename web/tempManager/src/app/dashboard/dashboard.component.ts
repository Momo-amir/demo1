import { Component, OnInit } from '@angular/core';
import Chart from 'chart.js/auto';

@Component({
  selector: 'app-dashboard',
  standalone: true,
  imports: [],
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.scss'],
})
export class DashboardComponent implements OnInit {
  tempChart: any = null;

  ngOnInit() {
    try {
      const canvas = document.getElementById('tempChart') as HTMLCanvasElement;
      if (canvas) {
        const ctx = canvas.getContext('2d');
        if (ctx && typeof Chart !== 'undefined') {
          this.tempChart = new Chart(ctx, {
            type: 'bar',
            data: {
              // Dummy data for preview
              labels: ['Jan', 'Feb', 'Mar', 'Apr', 'May'],
              datasets: [
                {
                  label: 'Temperature (°C)',
                  data: [10, 20, 15, 25, 30],
                  backgroundColor: [
                    'rgba(255, 99, 132, 0.2)',
                    'rgba(54, 162, 235, 0.2)',
                    'rgba(255, 206, 86, 0.2)',
                    'rgba(75, 192, 192, 0.2)',
                    'rgba(153, 102, 255, 0.2)',
                  ],
                },
              ],
            },
            options: {
              scales: {
                x: { title: { display: true, text: 'Time' } },
                y: { title: { display: true, text: 'Temperature (°C)' } },
              },
            },
          });
        } else {
          throw new Error(
            'Chart is not defined or canvas context not available.'
          );
        }
      } else {
        console.warn("No canvas element found with id 'tempChart'.");
      }
    } catch (err) {
      console.error(
        'Chart initialization failed - running without chart:',
        err
      );
    }
  }
}
